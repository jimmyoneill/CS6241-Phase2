#ifndef BOUNDSCHECKCODEGENERATOR_H
#define BOUNDSCHECKCODEGENERATOR_H

#include "llvm/Module.h"
#include "llvm/Instructions.h"
#include "ArrayAccess.h"

using namespace llvm;
using namespace std;
using namespace patterns;

namespace codegen
{
	class BoundsCheckCodeGenerator
	{
	private:
		Module *module;
		string checkFuncName;

	public:
		BoundsCheckCodeGenerator(Module *mod);
		string getCheckFuncName();
		void insertFuncDefinitions();
		CallInst *insertArrayBoundsCheck(GetElementPtrInst *gep, ArrayAccess *access);
		CallInst *insertArrayBoundsCheck(GetElementPtrInst *gep, vector<Value*> args);
	};

	string mangleName(string toMangle, const int length);
}

#endif
