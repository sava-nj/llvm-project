#include "llvm/Transforms/Utils/DebugCountPass.h"
#include "llvm/IR/InstIterator.h"  // For instruction iterator
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugProgramInstruction.h"

using namespace llvm;

PreservedAnalyses DebugCounterPass::run(Function &F, FunctionAnalysisManager &AM)
{
    unsigned dbgDeclareCounter {0}, dbgValueCounter {0}, dbgAssignCounter {0};

    for(Instruction &I : instructions(F))
    {
        for(DbgRecord &DbgRec : I.getDbgRecordRange())
        {
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

PreservedAnalyses RemoveDebugRecords::run(Function &F, FunctionAnalysisManager &AM)
{
     SmallVector<DbgVariableRecord*, 0> toErase;

    for(Instruction &I : instructions(F))
    {
        for(DbgRecord &DbgRec : I.getDbgRecordRange())
        {

            if(auto *DVR = dyn_cast<DbgVariableRecord>(&DbgRec))
            {
                
                if(DVR->isDbgDeclare() || DVR->isDbgValue() || DVR->isDbgAssign())
                {
                    toErase.push_back(DVR);
                }
            }
        }
    }

    for(DbgVariableRecord* DVR : toErase)
    {
        DVR->eraseFromParent();
    }

    return PreservedAnalyses::all();
}