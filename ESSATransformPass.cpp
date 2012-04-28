#include "ESSATransformPass.h"

namespace abcd
{
	ESSATransformPass::ESSATransformPass() : ModulePass(ID) {};

	bool ESSATransformPass::runOnModule(Module &M)
	{

	}

	void ESSATransformPass::getAnalysisUsage(AnalysisUsage &AU) const
	{

	}

	void ESSATransformPass::print(raw_ostream &O, const Module *M) const
	{

	}
}

char abcd::ESSATransformPass::ID = 0;
static RegisterPass<abcd::ESSATransformPass> X(
	"essa", 
	"CS6241 phase 2 eSSA IR transformation pass", 
	false, 
	false);
