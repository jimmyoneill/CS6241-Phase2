#include "BlockFrequencyPruner.h"

using namespace llvm;
namespace blockfreq {
    
    BlockFrequencyPruner::BlockFrequencyPruner(Module &m, ModulePass* owner) {
        this->m = &m;
        this->mp = owner;
    }
    
    void BlockFrequencyPruner::Init() {
        for (Module::iterator f = m->begin(); f != m->end(); f++) {
            if (!(*f).isDeclaration()) { 
                BlockFrequencyInfo &bfi = mp->getAnalysis<BlockFrequencyInfo>(*f);
                for (Function::iterator b = f->begin(); b != f->end(); b++) {
                    BlockFrequency bf = bfi.getBlockFreq( (BasicBlock*)b );
                    unsigned freq = bfi.getBlockFreq( (BasicBlock*)b ).getFrequency();
                    errs() << "basic block " << b->getName() << " has freq " << freq << "\n";
                }	    
            }
        }
    }
}