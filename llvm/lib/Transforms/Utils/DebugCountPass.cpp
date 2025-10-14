#include "llvm/Transforms/Utils/DebugCountPass.h"
#include "llvm/IR/InstIterator.h"  // For instruction iterator
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;

PreservedAnalyses DebugCounterPass::run(Function &F, FunctionAnalysisManager &AM)
{
    unsigned dbgDeclareCounter {0}, dbgValueCounter {0}, dbgAssignCounter {0};

    for(Instruction &I : instructions(F))
    {
        if(DbgVariableIntrinsic* DVI = dyn_cast<DbgVariableIntrinsic>(&I)) 
        // If instruction is not DbgVariableIntrinsic DVI will be nullptr => skip
        {
            auto id = DVI->getIntrinsicID();
            if(id == Intrinsic::dbg_declare)
            {
                ++dbgDeclareCounter;
            }
            else if(id == Intrinsic::dbg_value)
            {
                ++dbgValueCounter;
            }
            else if(id == Intrinsic::dbg_assign)
            {
                ++dbgAssignCounter;
            }
            
        }
    }

    outs() << "Function: " << F.getName() << "\n";
    outs() << "\tllvm.dbg.declare: " << dbgDeclareCounter << "\n";
    outs() << "\tllvm.dbg.value: " << dbgValueCounter << "\n";
    outs() << "\tllvm.dbg.assign: " << dbgAssignCounter << "\n";

    return PreservedAnalyses::all();
}