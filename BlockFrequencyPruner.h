#ifndef BLOCKFREQUENCYPRUNER_H
#define BLOCKFREQUENCYPRUNER_H

#include "ESSA.h"
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

        BlockFrequencyPruner(Module &m, ModulePass* owner, double massThreshold);
        std::vector<BasicBlock*> getBlocks();
        void init();
        
    private:
        
        std::vector<BasicBlock*> prunedBlocks;
        struct freqPredicate {
            std::map<BasicBlock*, unsigned> freqs;
            bool operator() (BasicBlock* b1, BasicBlock* b2) {
                return (freqs[b1] > freqs[b2]);
            }
        } _freqPredicate;
        
        unsigned freqSum;
        std::vector<double> getBlockDistribution(std::vector<BasicBlock*> blocks, double freqSum); 

        Module *m;
        ModulePass *mp;
        double massThreshold;
        unsigned blockCutoff;
    };
    
}

#endif
