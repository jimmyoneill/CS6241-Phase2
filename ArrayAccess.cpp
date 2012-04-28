#include "ArrayAccess.h"

namespace patterns
{
	ArrayAccess::ArrayAccess(GetElementPtrInst *gepInst, AllocaInst *allocaInst, Value *idx, int length)
	{
		this->gepInst = gepInst;
		this->allocaInst = allocaInst;
		this->index = idx;
		this->length = length;
	}

	int ArrayAccess::getLength()
	{
		return this->length;
	}
	
	Value *ArrayAccess::getIndex()
	{
		return this->index;
	}

	GetElementPtrInst *ArrayAccess::getGepInst()
	{
		return this->gepInst;
	}

	AllocaInst *ArrayAccess::getAllocaInst()
	{
		return this->allocaInst;
	}

	string ArrayAccess::getName()
	{
		return this->allocaInst != NULL ? this->allocaInst->getName() : NULL;
	}

	void ArrayAccess::dump(raw_ostream &O, bool printNewLine)
	{
		O << "Accessing a dimension of array ";
		O << this->getName() << " of size " << this->getLength();
		O << " by index " << *(this->index);

		if(printNewLine)
		{
			O << "\n";
		}
	}
}
