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
		Value *index;
		GetElementPtrInst *gepInst;
		AllocaInst *allocaInst;
		ArrayAccess(GetElementPtrInst *gepInst, AllocaInst *allocaInst, Value *idx, int length);
	
	public:
		int getLength();
		Value *getIndex();
		string getName();
		AllocaInst *getAllocaInst();
		GetElementPtrInst *getGepInst();
		void dump(raw_ostream &O, bool printNewLine = true);

		friend class ArrayAccessFactory;
	};
}

#endif
