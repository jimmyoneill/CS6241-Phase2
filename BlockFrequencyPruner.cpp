#include "BlockFrequencyPruner.h"

using namespace llvm;
namespace blockfreq {
    
    BlockFrequencyPruner::BlockFrequencyPruner(Module &m, ModulePass* owner) {
        this->m = &m;
        this->mp = owner;
    }
    
    void BlockFrequencyPruner::init() {
        
        std::vector<BasicBlock*> blocks;
        for (Module::iterator f = m->begin(); f != m->end(); f++) {
            if (!(*f).isDeclaration()) { 
                BlockFrequencyInfo &bfi = mp->getAnalysis<BlockFrequencyInfo>(*f);
                for (Function::iterator b = f->begin(); b != f->end(); b++) {
                    BlockFrequency bf = bfi.getBlockFreq( (BasicBlock*)b );
                    unsigned freq = bfi.getBlockFreq( (BasicBlock*)b ).getFrequency();
                    errs() << "basic block " << b->getName() << " has freq " << freq << "\n";
                    blocks.push_back(b);
                }	    
            }
        }
    }
    
    std::vector<BasicBlock*> BlockFrequencyPruner::getBlocks() {
        
        int count = 0;
        std::vector<BasicBlock*> blocks;
        for (Module::iterator f = m->begin(); f != m->end(); f++) {
            if (!(*f).isDeclaration()) { 
                BlockFrequencyInfo &bfi = mp->getAnalysis<BlockFrequencyInfo>(*f);
                for (Function::iterator b = f->begin(); b != f->end(); b++) {
                    BlockFrequency bf = bfi.getBlockFreq( (BasicBlock*)b );
                    unsigned freq = bfi.getBlockFreq( (BasicBlock*)b ).getFrequency();
                    errs() << "basic block " << b->getName() << " has freq " << freq << "\n";
                    blocks.push_back(b);
                    count++;
                    if (count > 1) return blocks;
                }	    
            }
        }
        
        return blocks;

    }
}

