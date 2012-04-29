#ifndef REDUNDANTCHECKELIMINATOR_H
#define REDUNDANTCHECKELIMINATOR_H

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instructions.h"
#include "ArrayAccess.h"
#include "BoundsCheckVisitor.h"
#include <vector>

using namespace llvm;
using namespace std;
using namespace patterns;

namespace patterns
{
	class RedundantCheckEliminator
	{
	private:
		static bool contains(vector<CallInst*> *vector, CallInst *inst);
		static bool subsumes(ArrayAccess *base, ArrayAccess *target, vector<StoreInst*> stores);
		
	public:
		static void process(BasicBlock *B, AccessMap accessMap);
	};

}

#endif
