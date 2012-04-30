#include "ConstraintBuilderPass.h"
#include "NaiveBoundsCheckInserter.h"
#include "ESSATransformPass.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "BlockFrequencyPruner.h"

using namespace nbci;
using namespace std;
using namespace GraphConstruct;
using namespace blockfreq;

namespace abcd
{
	ConstraintBuilderPass::ConstraintBuilderPass() : ModulePass(ID) 
	{
		this->removed = 0;
	}

	bool ConstraintBuilderPass::runOnModule(Module &M)
	{
        
        int totalChecks = 0;
        
		ESSATransformPass &essa = getAnalysis<ESSATransformPass>();
		NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();
        
        BlockFrequencyPruner *bfp = new BlockFrequencyPruner(M, this);
        bfp->init();
        std::vector<BasicBlock*> blocks = bfp->getBlocks();

		vector<CGGraph*> graphs = essa.getTransformedIr()->findConstraints(M, nbci);
	    vector<CGGraph*>::iterator graphIt;
		CGGraph* graph;

		for (graphIt = graphs.begin(); graphIt != graphs.end(); ++graphIt) 
		{
			graph = *graphIt;
			this->removed += (*graphIt)->solve(nbci.getBoundsCheckVisitor()->getAllCheckCalls());

			totalChecks += (*graphIt)->totalChecked;
            //this->removed += (*graphIt)->solve(nbci.getBoundsCheckVisitor()->getCheckCallsInBlocks(blocks));
		}

		errs() << "Total number of checks: " << totalChecks << "\n";
		errs() << "Total number of checks removed: " << this->removed << "\n";
         
		return this->removed > 0;
	}

	void ConstraintBuilderPass::getAnalysisUsage(AnalysisUsage &AU) const
	{
        AU.setPreservesCFG();
		AU.addRequired<ESSATransformPass>();
		AU.addRequired<NaiveBoundsCheckInserter>();
        AU.addRequired<BlockFrequencyInfo>();
        AU.addPreserved<BlockFrequencyInfo>();
    }
}

char abcd::ConstraintBuilderPass::ID = 0;


static RegisterPass<abcd::ConstraintBuilderPass> X(
	"abcd", 
	"CS6241 Phase2 ABCD constraint graph check reduction pass", 
	false,
	false);
