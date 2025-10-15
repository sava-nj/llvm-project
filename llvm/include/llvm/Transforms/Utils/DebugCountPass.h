#ifndef DEBUG_COUNT_PASS_H
#define DEBUG_COUNT_PASS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class DebugCounterPass : public PassInfoMixin<DebugCounterPass> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


class RemoveDebugRecords : public PassInfoMixin<RemoveDebugRecords> {
public:
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
};


} // namespace llvm


#endif