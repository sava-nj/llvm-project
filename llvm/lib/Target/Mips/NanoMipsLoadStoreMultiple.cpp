//===- NanoMipsLoadStoreMultiple.cpp - nanoMIPS load / store opt. pass
//--------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file This file contains a pass that performs load / store related peephole
/// optimizations. This pass should be run after register allocation.
//
//===----------------------------------------------------------------------===//

#include "Mips.h"
#include "MipsSubtarget.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/InitializePasses.h"

#include <cmath>

using namespace llvm;

#define DEBUG_TYPE "nanomips-lwm-swm"
#define NM_LOAD_STORE_OPT_NAME "nanoMIPS load/store multiple optimization pass"

static cl::opt<bool> DisableNMLoadStoreMultiple(
    "disable-nm-lwm-swm", cl::Hidden, cl::init(false),
    cl::desc("Disable NanoMips load/store multiple optimizations"));

namespace {
struct NMLoadStoreMultipleOpt : public MachineFunctionPass {
  using MBBIter = MachineBasicBlock::iterator;   
  static char ID;
  const MipsSubtarget *STI;
  const TargetInstrInfo *TII;
  MCRegisterClass RC = MipsMCRegisterClasses[Mips::GPRNM32RegClassID];
  DenseMap<unsigned, unsigned> RegToIndexMap;

  NMLoadStoreMultipleOpt() : MachineFunctionPass(ID) {
    // Initialize RegToIndexMap.
    for (unsigned I = 0; I < RC.getNumRegs(); I++) {
      unsigned R = RC.begin()[I];
      RegToIndexMap[R] = I;
    }
  }
  StringRef getPassName() const override { return NM_LOAD_STORE_OPT_NAME; }
  bool runOnMachineFunction(MachineFunction &Fn) override;
  unsigned getRegNo(unsigned Reg);
  bool isValidLoadStore(MachineInstr &MI, bool IsLoad);
  bool generateLoadStoreMultiple(MachineBasicBlock &MBB, bool IsLoad, unsigned MaxSeq);
  void sortCandidatesBasedOnOffset(SmallVector<MachineInstr *> &LoadStoreList);
  void saveValidSequence(int &beginIndex, int &endIndex, const SmallVector<MachineInstr *> &Candidate, 
    SmallVector<SmallVector<MachineInstr *>> &LoadStoreMultipleCandidates);
};
} // namespace

char NMLoadStoreMultipleOpt::ID = 0;

bool NMLoadStoreMultipleOpt::runOnMachineFunction(MachineFunction &Fn) {
  STI = &static_cast<const MipsSubtarget &>(Fn.getSubtarget());
  TII = STI->getInstrInfo();
  if (DisableNMLoadStoreMultiple || !Fn.getFunction().hasOptSize())
   return false;
  bool Modified = false;
  for (MachineFunction::iterator MFI = Fn.begin(), E = Fn.end(); MFI != E;
       ++MFI) {
    MachineBasicBlock &MBB = *MFI;
    Modified |= generateLoadStoreMultiple(MBB, /*IsLoad=*/false, /*MaxSeq=*/8);
    Modified |= generateLoadStoreMultiple(MBB, /*IsLoad=*/true, /*MaxSeq=*/8);
  }

  return Modified;
}

unsigned NMLoadStoreMultipleOpt::getRegNo(unsigned Reg) {
  auto I = RegToIndexMap.find(Reg);

  // Invalid register index.
  if (I == RegToIndexMap.end())
    return RC.getNumRegs();

  return I->second;
}

bool NMLoadStoreMultipleOpt::isValidLoadStore(MachineInstr &MI, bool IsLoad) {
  unsigned Opcode = MI.getOpcode();
  // Make sure the instruction doesn't have any atomic, volatile or
  // otherwise strictly ordered accesses.
  for (auto &MMO : MI.memoperands())
    if (MMO->isAtomic() || !MMO->isUnordered())
      return false;

  Register Rt, Rs;
  if (IsLoad) {
    // TODO: Handle unaligned loads and stores.
    if (Opcode == Mips::LW_NM || Opcode == Mips::LWs9_NM) {
      // TODO: Rt and Rs can be equal, but only if that is the last load of the sequence.
      Register Rt = MI.getOperand(0).getReg();
      Register Rs = MI.getOperand(1).getReg();
      if (Rt != Rs)
        return true;
    }
  } else {
    if (Opcode == Mips::SW_NM || Opcode == Mips::SWs9_NM)
      return true;
  }
  return false;
}

void NMLoadStoreMultipleOpt::sortCandidatesBasedOnOffset(SmallVector<MachineInstr *> &LoadStoreList) {
  auto CompareInstructions = [this](MachineInstr *First, MachineInstr *Second) {
    // Compare first with second MI offset
    return First->getOperand(2).getImm() < Second->getOperand(2).getImm();
  };
  std::sort(LoadStoreList.begin(), LoadStoreList.end(), CompareInstructions);
}

void NMLoadStoreMultipleOpt::saveValidSequence(int &beginIndex, int &endIndex, const SmallVector<MachineInstr *> &Candidate, 
  SmallVector<SmallVector<MachineInstr *>> &LoadStoreMultipleCandidates) {
  SmallVector<MachineInstr *> SubSequence;
  for (auto i = beginIndex; i <= endIndex; ++i) {
    SubSequence.push_back(Candidate[i]);
  }
  LoadStoreMultipleCandidates.push_back(std::move(SubSequence));
  beginIndex = -1;
  endIndex = -1;
}


bool NMLoadStoreMultipleOpt::generateLoadStoreMultiple(MachineBasicBlock &MBB,
                                                       bool IsLoad, unsigned MaxSeq) { 
  SmallVector<SmallVector<MachineInstr *>> Candidates;    // Vector of potential LW/SW consecutive sequences
  SmallVector<MachineInstr *> Sequence;                   // Sequence of consecutive LW/SW instructions
  bool Modified = false;
  // Iterate through machine instructions in the machine basic block and 
  // collect consecutive LW/SW with same source register into Candidates vector.
  for (auto &MI : MBB) {
    // CFI and debug instructions don't break the sequence.
    if (MI.isCFIInstruction() || MI.isDebugInstr())
      continue;

    if (isValidLoadStore(MI, IsLoad)) {
      if (!Sequence.empty()) {
        // If the sequence is not empty, check whether the source registers of the current and previous LW/SW instructions match.
        auto PrevMI = Sequence.back();
        if (MI.getOperand(1).getReg().id() != PrevMI->getOperand(1).getReg().id()) {
          if (Sequence.size() > 1) {
            // If source registers differ and the current sequence has at least two MI, save it as a candidate
            Candidates.push_back(Sequence);
          }
          Sequence.clear();
        }
      }
      // Push MI into the sequence if the sequence is empty or if the
      // source register matches the previous instruction's source register.
      Sequence.push_back(&MI);
    }  
    else {
      if (Sequence.size() > 1) {
        // On any non-LW/SW instruction, save sequence if it contains at least two instructions.
        Candidates.push_back(Sequence);
      }
      Sequence.clear();
    }
  }

  // Make sure that the last sequence has been added to the Candidates list.
  if (Sequence.size() > 1)
    Candidates.push_back(Sequence);

  // Sequence to change with LWM/SWM
  SmallVector<SmallVector<MachineInstr *>> LoadStoreMultipleCandidates;

  // Candidates consist of potential optimizable LW/SW consecutive instructions. 
  for (auto &Candidate : Candidates) {
    // Sort machine instructions in each candidate vector based on offset.
    sortCandidatesBasedOnOffset(Candidate);

    /* Print sorted sequences: */
    // errs() << "\n--- SEQ --- \n";
    // for (auto &MI : Candidate) {
    //   MI->dump();
    // }
    // errs() << "\n";

    auto beginIdx = -1, endIdx = -1;
    for (size_t idx = 0; idx < Candidate.size(); idx++) {
      MachineInstr *Current = Candidate[idx];
      int64_t CurrentOffset = Current->getOperand(2).getImm();
      unsigned CurrentRtNo = getRegNo(Current->getOperand(0).getReg().id());
      
      if (!isInt<9>(CurrentOffset))
        continue;

      MachineInstr *Next = (idx + 1 < Candidate.size()) ? Candidate[idx + 1] : nullptr;
      if (Next) {
        // Check if there are two valid, consecutive lw/sw instructions.
        int64_t NextOffset = Next->getOperand(2).getImm();
        unsigned NextDesiredRtNo = CurrentRtNo != 0 ? (CurrentRtNo == 31 ? 16 : CurrentRtNo + 1) : 0;
        if ((CurrentOffset == NextOffset - 4) && (NextDesiredRtNo == getRegNo(Next->getOperand(0).getReg().id()))) {
          if(beginIdx == -1) 
            beginIdx = idx;
          endIdx = idx + 1;
          
          // The maximum sequence size is 8 (example: indices from 0 to 7)
          if (endIdx - beginIdx == 7) {
            ++idx;  // The current MI will be included in the sequence, so skip it in the next iteration.
            saveValidSequence(beginIdx, endIdx, Candidate, LoadStoreMultipleCandidates); 
          }
          
        }
        else {
          // If a valid lw/sw sequence was detected, save it. And start searching for a new one.
          if (beginIdx != -1) 
            saveValidSequence(beginIdx, endIdx, Candidate, LoadStoreMultipleCandidates); 
        }
      }
      else {
        // If this is the last MI in sequence and a valid lw/sw sequence was detected, save it.
        if (beginIdx != -1) 
          saveValidSequence(beginIdx, endIdx, Candidate, LoadStoreMultipleCandidates);
      }
    }
  }

  for (auto &Seq : LoadStoreMultipleCandidates) {
    assert(Seq.size() > 1 && Seq.size() < 9);

    auto *Base = Seq.front();
    int64_t Offset = Base->getOperand(2).getImm();
    // Sequence cannot be merged, if the offset is out of range.
    if (!isInt<9>(Offset))
      continue;

    auto InsertBefore = std::next(MBBIter(Base));
    unsigned Opcode = IsLoad ? Mips::LWM_NM : Mips::SWM_NM;
    auto BMI =
        BuildMI(MBB, InsertBefore, Base->getDebugLoc(), TII->get(Opcode))
            .addReg(Base->getOperand(0).getReg(), IsLoad ? RegState::Define : 0)
            .addReg(Base->getOperand(1).getReg())
            .addImm(Offset)
            .addImm(Seq.size());
    BMI.cloneMergedMemRefs(Seq);
    for (auto *MI : Seq) {
      if (MI != Base)
        BMI.addReg(MI->getOperand(0).getReg(),
                   IsLoad ? RegState::ImplicitDefine : RegState::Implicit);
      MBB.erase(MI);
    }

    /* Print new SWM/LWM instruction: */
    // errs() << "\n--- SWM/LWM --- \n";
    // BMI.getInstr()->dump();

    Modified = true;
  }
  return Modified;
}

INITIALIZE_PASS(NMLoadStoreMultipleOpt, DEBUG_TYPE, NM_LOAD_STORE_OPT_NAME,
                false, false)

namespace llvm {
FunctionPass *createNanoMipsLoadStoreMultiplePass() {
  return new NMLoadStoreMultipleOpt();
}
} // namespace llvm
