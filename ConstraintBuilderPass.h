#ifndef CONSTRAINTBUILDERPASS_H
#define CONSTRAINTBUILDERPASS_H

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
	struct ConstraintBuilderPass : public ModulePass
	{
	public:
		static char ID;
		ConstraintBuilderPass();
		virtual bool runOnModule(Module &M);
		void getAnalysisUsage(AnalysisUsage &AU) const;
		virtual void print(raw_ostream &O, const Module *M) const;
	};
}

#endif
