#include "ESSATransformPass.h"

using namespace nbci;

namespace abcd
{
	ESSATransformPass::ESSATransformPass() : ModulePass(ID) {}

	bool ESSATransformPass::runOnModule(Module &M)
	{
		NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();
		this->transformedIr = new ESSA(M, nbci.getCheckFuncName());
		for (Module::iterator func = M.begin(); func != M.end(); func++) 
		{
			//rename variables in ESSA
			if (!(func->empty()|| func->isDeclaration())) 
			{
				this->transformedIr->rename(getAnalysis<DominatorTree>(*func));
			}

	 	}

		return false;
	}

	void ESSATransformPass::getAnalysisUsage(AnalysisUsage &AU) const
	{
		AU.addRequired<DominatorTree>();
		AU.addRequired<NaiveBoundsCheckInserter>();
		AU.setPreservesAll();
	}

	ESSA *ESSATransformPass::getTransformedIr()
	{
		return this->transformedIr;
	}
}

char abcd::ESSATransformPass::ID = 0;
static RegisterPass<abcd::ESSATransformPass> X(
	"essa", 
	"CS6241 Phase2 ESSA IR transformation pass", 
	false, 
	false);
