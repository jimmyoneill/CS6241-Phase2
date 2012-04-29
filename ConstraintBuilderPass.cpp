#include "ConstraintBuilderPass.h"
#include "NaiveBoundsCheckInserter.h"
#include "ESSATransformPass.h"

using namespace nbci;
using namespace std;
using namespace GraphConstruct;

namespace abcd
{
	ConstraintBuilderPass::ConstraintBuilderPass() : ModulePass(ID) {}

	bool ConstraintBuilderPass::runOnModule(Module &M)
	{
		ESSATransformPass &essa = getAnalysis<ESSATransformPass>();
		NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();

		vector<CGGraph*> graphs = essa.getTransformedIr()->find_constraints(M, nbci);
	    vector<CGGraph*>::iterator graphIt;
		CGGraph* graph;

		for (graphIt = graphs.begin(); graphIt != graphs.end(); ++graphIt) 
		{
			graph = *graphIt;
			(*graphIt)->solve(nbci.getBoundsCheckVisitor()->getAllCheckCalls());
		}

		return true;
	}

	void ConstraintBuilderPass::getAnalysisUsage(AnalysisUsage &AU) const
	{
		AU.addRequired<ESSATransformPass>();
		AU.addRequired<NaiveBoundsCheckInserter>();
		AU.setPreservesCFG();
	}
}

char abcd::ConstraintBuilderPass::ID = 0;
static RegisterPass<abcd::ConstraintBuilderPass> X(
	"abcd", 
	"CS6241 Phase2 ABCD constraint graph check reduction pass", 
	false,
	false);
