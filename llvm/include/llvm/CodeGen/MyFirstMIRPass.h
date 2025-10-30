#ifndef LLVM_CODEGEN_MYFIRSTMIRPASS_H
#define LLVM_CODEGEN_MYFIRSTMIRPASS_H

#include "llvm/CodeGen/MachinePassManager.h"

namespace llvm {
class InstructionCounterPass : public PassInfoMixin<InstructionCounterPass> {
public : 
    PreservedAnalyses run(MachineFunction &MF, MachineFunctionAnalysisManager &MFAM);
};

} // namespace llvm

# endif // LLVM_CODEGEN_MYFIRSTFUNCTIONPASS_H