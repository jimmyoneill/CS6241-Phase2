#include "BoundsCheckVisitor.h"
#include "ArrayAccessFactory.h"
#include "llvm/Constants.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Support/IRBuilder.h"
#include <stack>

namespace patterns
{
	BoundsCheckVisitor::BoundsCheckVisitor(BoundsCheckCodeGenerator *codeGen)
	{
		this->changed = false;
		this->codeGen = codeGen;
	}

	void BoundsCheckVisitor::visitGetElementPtrInst(GetElementPtrInst *inst)
	{	
		ArrayAccess *access = ArrayAccessFactory::create(inst);
		if(access != NULL)
		{
			CallInst *call = this->codeGen->insertArrayBoundsCheck(inst, access);
			if(call != NULL)
			{
				this->accessMap[call] = access; // Store it for bookkeeping.
				this->checkCalls.push_back(call);
				this->changed = true;
			}
		}
	}

	AccessMap BoundsCheckVisitor::getArrayAccessMap()
	{
		return this->accessMap;
	}

	CheckCalls BoundsCheckVisitor::getAllCheckCalls()
	{
		return this->checkCalls;
	}

	bool BoundsCheckVisitor::insertedChecks()
	{
		return this->changed;
	}

	BoundsCheckCodeGenerator *BoundsCheckVisitor::getCodeGenerator()
	{
		return this->codeGen;
	}
}
