#include "llvm/Transforms/Utils/DebugCountPass.h"
#include "llvm/IR/InstIterator.h"  // For instruction iterator
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugProgramInstruction.h"

using namespace llvm;

PreservedAnalyses DebugCounterPass::run(Function &F, FunctionAnalysisManager &AM)
{
    unsigned dbgDeclareCounter {0}, dbgValueCounter {0}, dbgAssignCounter {0};
    
    F.print(errs());
    errs() << "====================================\n";

    for(Instruction &I : instructions(F))
    {
        for(DbgRecord &DbgRec : I.getDbgRecordRange())
        {
            errs() << DbgRec << "\n";

            if(auto *DVR = dyn_cast<DbgVariableRecord>(&DbgRec))
            {
                if(DVR->isDbgDeclare())
                {
                    ++dbgDeclareCounter;
                }
                else if(DVR->isDbgValue())
                {
                    ++dbgValueCounter;
                }
                else if(DVR->isDbgAssign())
                {
                    ++dbgAssignCounter;
                }
            }
        }
    }

    errs() << "Function: " << F.getName() << "\n";
    errs() << "\tllvm.dbg.declare: " << dbgDeclareCounter << "\n";
    errs() << "\tllvm.dbg.value: " << dbgValueCounter << "\n";
    errs() << "\tllvm.dbg.assign: " << dbgAssignCounter << "\n";

    return PreservedAnalyses::all();
}