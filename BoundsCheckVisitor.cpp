#include "BoundsCheckVisitor.h"
#include "ArrayAccessFactory.h"
#include "RedundantCheckEliminator.h"
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

	void BoundsCheckVisitor::visitBasicBlock(BasicBlock *blk)
	{
		// call super class's visit
		InstructionVisitor::visitBasicBlock(blk);

		// Eliminate some redundant checks from that block.
		if(REDUNDANT_CHECK_ELIMINATION)
		{
			vector<CallInst*> redundantChks;
			RedundantCheckEliminator::getRedundantCheckCalls(blk, this->accessMap, redundantChks);

			if(redundantChks.size() > 0)
			{
				this->eliminatedChecks += redundantChks.size();
				for(vector<CallInst*>::iterator i = redundantChks.begin(); i != redundantChks.end(); i++)
				{
					this->checkCalls.erase(remove(this->checkCalls.begin(), this->checkCalls.end(), *i));
					this->accessMap.erase(*i);
					(*i)->eraseFromParent();
				}
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

	int BoundsCheckVisitor::getNumEliminatedChecks()
	{
		return this->eliminatedChecks;
	}
}
