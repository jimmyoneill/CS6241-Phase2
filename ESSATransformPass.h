#ifndef ESSATRANSFORMPASS_H
#define ESSATRANSFORMPASS_H

#include "eSSA.h"
#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "BoundsCheckVisitor.h"
#include "BoundsCheckCodeGenerator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace abcd
{
	struct ESSATransformPass : public ModulePass
	{
	private:
		eSSA *transformedIr;
	public:
		static char ID;
		ESSATransformPass();
		virtual bool runOnModule(Module &M);
		void getAnalysisUsage(AnalysisUsage &AU) const;
		eSSA *getTransformedIr();
	};
}

#endif
