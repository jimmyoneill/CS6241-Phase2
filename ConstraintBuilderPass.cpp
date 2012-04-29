#include "ConstraintBuilderPass.h"
#include "NaiveBoundsCheckInserter.h"
#include "ESSATransformPass.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"

using namespace nbci;
using namespace std;
using namespace GraphConstruct;

namespace abcd
{
	ConstraintBuilderPass::ConstraintBuilderPass() : ModulePass(ID) 
	{
		this->removed = 0;
	}

	bool ConstraintBuilderPass::runOnModule(Module &M)
	{
        
        /*
        errs() << "test1\n";
        
        for (Module::iterator f = M.begin(); f != M.end(); f++) {
            if (!(*f).isDeclaration()) { 
            BlockFrequencyInfo &bfi = getAnalysis<BlockFrequencyInfo>(*f);
                
            for (Function::iterator b = f->begin(); b != f->end(); b++) {
                errs() << "test1\n";
                errs() << "bfi = " << &bfi;
                BlockFrequency bf = bfi.getBlockFreq( (BasicBlock*)b );
                errs() << "test2\n";
                unsigned freq = bfi.getBlockFreq( (BasicBlock*)b ).getFrequency();
                errs() << "test3\n";
                errs() << "basic block " << b->getName() << " has freq " << freq << "\n";
            }	    
            }
        }
        */
        
		ESSATransformPass &essa = getAnalysis<ESSATransformPass>();
		NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();

		vector<CGGraph*> graphs = essa.getTransformedIr()->findConstraints(M, nbci);
	    vector<CGGraph*>::iterator graphIt;
		CGGraph* graph;

		for (graphIt = graphs.begin(); graphIt != graphs.end(); ++graphIt) 
		{
			graph = *graphIt;
			this->removed += (*graphIt)->solve(nbci.getBoundsCheckVisitor()->getAllCheckCalls());
		}

		errs() << "Total number of checks removed: " << this->removed << "\n";
         
		return this->removed > 0;
	}

	void ConstraintBuilderPass::getAnalysisUsage(AnalysisUsage &AU) const
	{
        AU.setPreservesCFG();
		AU.addRequired<ESSATransformPass>();
		AU.addRequired<NaiveBoundsCheckInserter>();
        //AU.addRequired<BlockFrequencyInfo>();
        //AU.addPreserved<BlockFrequencyInfo>();
    }
}

char abcd::ConstraintBuilderPass::ID = 0;


static RegisterPass<abcd::ConstraintBuilderPass> X(
	"abcd", 
	"CS6241 Phase2 ABCD constraint graph check reduction pass", 
	false,
	false);
