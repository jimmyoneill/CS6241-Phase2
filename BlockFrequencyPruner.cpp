#include "BlockFrequencyPruner.h"

using namespace llvm;
namespace blockfreq {
    
    
    BlockFrequencyPruner::BlockFrequencyPruner(Module &m, ModulePass* owner, double massThreshold = 1) {
        this->m = &m;
        this->mp = owner;
        this->massThreshold = massThreshold;
    }
    
    void BlockFrequencyPruner::init() {
        
        freqSum = 0;
        std::vector<BasicBlock*> blocks;
        for (Module::iterator f = m->begin(); f != m->end(); f++) {
            if (!(*f).isDeclaration()) { 
                BlockFrequencyInfo &bfi = mp->getAnalysis<BlockFrequencyInfo>(*f);
                for (Function::iterator b = f->begin(); b != f->end(); b++) {
                    BasicBlock *block = b;
                    unsigned freq = bfi.getBlockFreq(block).getFrequency();
                    freqSum += freq;
                    blocks.push_back(block);
                    _freqPredicate.freqs[block] = freq;
                }	    
            }
        }
        
        std::sort(blocks.begin(), blocks.end(), _freqPredicate);
        std::vector<double> dist = getBlockDistribution(blocks, (double)freqSum);
        double distSum = 0;
        
        for (unsigned i = 0; (i < blocks.size()) && (distSum < massThreshold); ++i, distSum += dist.at(i)) {
            prunedBlocks.push_back(blocks.at(i));
        }
        
    }
    
    std::vector<double> BlockFrequencyPruner::getBlockDistribution(std::vector<BasicBlock*> blocks, double freqSum) {
        
        std::vector<double> dist;
        std::vector<BasicBlock*>::iterator it;
        double mass;
        for (it = blocks.begin(); it != blocks.end(); ++it) {
            mass = ((double)_freqPredicate.freqs[*it]) / freqSum;
            dist.push_back(mass);
        }
        return dist;
    }
    
    std::vector<BasicBlock*> BlockFrequencyPruner::getBlocks() {
        return prunedBlocks;
    }
}

