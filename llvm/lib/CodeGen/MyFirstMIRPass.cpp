#include "llvm/CodeGen/MyFirstMIRPass.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/InitializePasses.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;

class InstructionCounter : public MachineFunctionPass {
public: 
    static char ID;

    InstructionCounter() : MachineFunctionPass(ID) {
        initializeInstructionCounterPass(*PassRegistry::getPassRegistry());
    }

    bool runOnMachineFunction(MachineFunction &MF) override {          
        unsigned loadNo {0}, storeNo {0}, addNo {0};

        errs() << "Function name: " << MF.getName() << "\n";
        
        const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();

        for(MachineBasicBlock &MBB : MF) {
            for(MachineInstr &MI : MBB) {
                auto InstrOpcode = MI.getOpcode();
                StringRef instrName = TII->getName(InstrOpcode);
                // errs() << instrName << "\n";

                if(instrName.starts_with("MOV") && (instrName.ends_with("mr") || instrName.ends_with("mi"))) {
                    ++storeNo;
                }
                else if(instrName.starts_with("MOV") && instrName.ends_with("rm")) {
                    ++loadNo;
                }
                else if(instrName.starts_with("ADD")) {
                    ++addNo;
                }
                
            }
        }
        errs() << "\t" << "- load: " << loadNo << "\n";
        errs() << "\t" << "- store: " << storeNo << "\n";
        errs() << "\t" << "- add: " << addNo << "\n";
        errs() << "----------------------" << "\n";
        return false;
    }

};


PreservedAnalyses InstructionCounterPass::run(MachineFunction &MF, MachineFunctionAnalysisManager &MFAM) {    
    return PreservedAnalyses::all();
}

char InstructionCounter::ID = 0;

INITIALIZE_PASS(InstructionCounter, "my-pass", "My first machine function pass", true, true);
