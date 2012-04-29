#ifndef BLOCKFREQUENCYPRUNER_H
#define BLOCKFREQUENCYPRUNER_H

#include "eSSA.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "BoundsCheckVisitor.h"
#include "BoundsCheckCodeGenerator.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"

using namespace llvm;

namespace blockfreq 
{
    class BlockFrequencyPruner {
        
        public:
            BlockFrequencyPruner(Module &m, ModulePass* owner);
            void Init();
            
        private:
            Module *m;
            ModulePass *mp;
    };

}

#endif
