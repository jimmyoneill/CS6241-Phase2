#include "RedundantCheckEliminator.h"
#include "NaiveBoundsCheckInserter.h"

using namespace llvm;
using namespace std;

namespace patterns
{
	void RedundantCheckEliminator::process(BasicBlock *B, AccessMap accessMap)
	{
		vector<CallInst*> subsumedChks;

		// Basic block local only
		for(BasicBlock::iterator bi = B->begin(); bi != B->end(); bi++)
		{
			CallInst *currentCheck, *nextCheck;
			ArrayAccess *currentAccess, *nextAccess;
			vector<StoreInst*> stores;

			if((currentCheck = dyn_cast<CallInst>(bi)) != NULL &&
			   (currentAccess = accessMap[currentCheck]) != NULL &&
			   !contains(&subsumedChks, currentCheck))
			{
				// Now search from end to here of the BB for other checks.
				// This is a HACK, but LLVM doesn't allow iterator arithmatic
				BasicBlock::iterator search = bi;
				for(++search; search != B->end(); search++)
				{
					if((nextCheck = dyn_cast<CallInst>(search)) != NULL &&
					   (nextAccess = accessMap[nextCheck]) != NULL &&
					   !contains(&subsumedChks, nextCheck) &&
					   subsumes(currentAccess, nextAccess, stores))
					{
						subsumedChks.push_back(nextCheck);
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

		// remove all subsumed checks
		for(vector<CallInst*>::iterator i = subsumedChks.begin(); i != subsumedChks.end(); i++)
		{
			(*i)->eraseFromParent();
		}

		errs() << "Redundant checks removed.";
	}

	bool RedundantCheckEliminator::contains(vector<CallInst*> *vector, CallInst *inst)
	{
		return vector != NULL &&
			   inst != NULL &&
			   find(vector->begin(), vector->end(), inst) != vector->end();
	}

	bool RedundantCheckEliminator::subsumes(ArrayAccess *base, ArrayAccess *target, vector<StoreInst*> store)
	{
		return true;
	}
}
