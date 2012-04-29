#include "ArrayAccess.h"

namespace patterns
{
	ArrayAccess::ArrayAccess(GetElementPtrInst *gepInst, AllocaInst *allocaInst, int dim, Value *idx, int length)
	{
		this->gepInst = gepInst;
		this->allocaInst = allocaInst;
		this->index = idx;
		this->length = length;
		this->dimension = dim;
	}

	int ArrayAccess::getLength() const
	{
		return this->length;
	}
	
	int ArrayAccess::getDimension() const
	{
		return this->dimension;
	}

	Value *ArrayAccess::getIndex() const
	{
		return this->index;
	}

	GetElementPtrInst *ArrayAccess::getGepInst() const
	{
		return this->gepInst;
	}

	AllocaInst *ArrayAccess::getAllocaInst() const
	{
		return this->allocaInst;
	}

	string ArrayAccess::getName() const
	{
		return this->allocaInst != NULL ? this->allocaInst->getName() : "Unnamed";
	}

	void ArrayAccess::dump(raw_ostream &O, bool printNewLine)
	{
		O << "Accessing dimension " << this->dimension << " of array ";
		O << this->getName() << " with length " << this->getLength();
		O << " by index " << *(this->index);

		if(printNewLine)
		{
			O << "\n";
		}
	}

	bool ArrayAccess::equals(ArrayAccess &other)
	{
		//errs() << "Dimension equal? " << (this->dimension == other.dimension) << "\n";
		//errs() << "Length equal? " << (this->length == other.length) << "\n";

		return this->dimension == other.dimension && 
			   this->length == other.length &&
			   accessesSameArray(*this, other) &&
			   usesSameIndexVariable(*(this->getIndex()), *(other.getIndex()));
	}

	bool accessesSameArray(ArrayAccess &first, ArrayAccess &second)
	{
		//errs() << "First equal to second alloca? " << (first.getAllocaInst() == second.getAllocaInst()) << "\n";
		return first.getAllocaInst() == second.getAllocaInst();
	}

	bool usesSameIndexVariable(Value &first, Value &second)
	{	
		//errs() << "Uses same index variable? " << true << "\n";
		return true;
	}
}
