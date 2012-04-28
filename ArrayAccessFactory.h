#ifndef ARRAYACCESSFACTORY_H
#define ARRAYACCESSFACTORY_H

#include "ArrayAccess.h"

namespace patterns
{
	class ArrayAccessFactory
	{
	private:
		ArrayAccessFactory() {};

	public:
		static ArrayAccess *create(GetElementPtrInst *gepInst);
	};

	AllocaInst *findAllocaInst(GetElementPtrInst *inst);
	Value *findIndex(GetElementPtrInst *inst);

	// Debugging
	void debugGep(GetElementPtrInst *inst);
}

#endif
