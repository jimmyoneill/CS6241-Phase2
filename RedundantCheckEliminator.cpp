#include "RedundantCheckEliminator.h"
#include "NaiveBoundsCheckInserter.h"

using namespace llvm;
using namespace std;

namespace patterns
{
	void RedundantCheckEliminator::getRedundantCheckCalls(BasicBlock *B, AccessMap accessMap, vector<CallInst*> redundantChks)
	{
		//vector<CallInst*> redundantChks;

		// Basic block local only
		for(BasicBlock::iterator bi = B->begin(); bi != B->end(); bi++)
		{
			CallInst *currentCheck, *nextCheck;
			ArrayAccess *currentAccess, *nextAccess;
			vector<StoreInst*> stores;

			if((currentCheck = dyn_cast<CallInst>(bi)) != NULL &&
			   (currentAccess = accessMap[currentCheck]) != NULL &&
			   !contains(&redundantChks, currentCheck))
			{
				// Now search from end to here of the BB for other checks.
				// This is a HACK, but LLVM doesn't allow iterator arithmatic
				BasicBlock::iterator search = bi;
				for(++search; search != B->end(); search++)
				{
					if((nextCheck = dyn_cast<CallInst>(search)) != NULL &&
					   (nextAccess = accessMap[nextCheck]) != NULL &&
					   !contains(&redundantChks, nextCheck) &&
					   subsumes(currentAccess, nextAccess, stores))
					{
						//errs() << "Check is redundant!\n";
						//errs() << "--------------------------------------\n";
						//nextCheck->dump();
						//errs() << "--------------------------------------\n";
						redundantChks.push_back(nextCheck);
					}
					else if (StoreInst *store = dyn_cast<StoreInst>(search))
					{
						// A store to a subscript could cause trouble
						stores.push_back(store);
					}
				}

				// clear the stores vector so it can be used again.
				stores.clear();
			}
		}
	}

	bool RedundantCheckEliminator::contains(vector<CallInst*> *vector, CallInst *inst)
	{
		return vector != NULL &&
			   inst != NULL &&
			   find(vector->begin(), vector->end(), inst) != vector->end();
	}

	bool RedundantCheckEliminator::subsumes(ArrayAccess *base, ArrayAccess *target, vector<StoreInst*> stores)
	{
		if(base == NULL || target == NULL)
		{
			return false;
		}

		/* 
		 * First, check to see if the base and target array accesses are the same.
		 * Equiavalence here refers to the following:
		 *   1. Dimensions are the same.
		 *   2. Target allocated arrays are the same. 
		 *   3. Indices refer to the same variable. 
		 *
		 * Next, check to see if the indices have changed in between the two accesses.
		 * If one of them has, then the first check will never subsume the second.
		 */


		//base->dump(errs());
		//target->dump(errs());
		return base->equals(*target) && !hasIndexChanged(base->getIndex(), stores);
	}

	bool RedundantCheckEliminator::hasIndexChanged(Value *value, vector<StoreInst*> stores)
	{
		//errs() << "Has index changed? " << false << "\n";
		return false;
	}
}
