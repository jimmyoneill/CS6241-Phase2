#ifndef ARRAYACCESS_H
#define ARRAYACCESS_H

#include "llvm/Instructions.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace patterns
{
	class ArrayAccess
	{
	protected:
		int length;
		int dimension;
		Value *index;
		GetElementPtrInst *gepInst;
		AllocaInst *allocaInst;
		ArrayAccess(GetElementPtrInst *gepInst, AllocaInst *allocaInst, int dim, Value *idx, int length);
	
	public:
		int getLength() const;
		int getDimension() const;
		Value *getIndex() const;
		string getName() const;
		AllocaInst *getAllocaInst() const;
		GetElementPtrInst *getGepInst() const;
		void dump(raw_ostream &O, bool printNewLine = true);
		bool equals(ArrayAccess &other);

		friend class ArrayAccessFactory;
	};

	bool accessesSameArray(ArrayAccess &first, ArrayAccess &second);
	bool usesSameIndexVariable(Value &first, Value &second);
}

#endif
