#ifndef BOUNDSCHECKVISITOR_H
#define BOUNDSCHECKVISITOR_H

#include "InstructionVisitor.h"
#include "ArrayAccess.h"
#include "BoundsCheckCodeGenerator.h"
#include <map>

using namespace llvm;
using namespace std;
using namespace codegen;

namespace patterns
{
	typedef map<CallInst*,ArrayAccess*> AccessMap;
	typedef vector<CallInst*> CheckCalls;

	class BoundsCheckVisitor : public InstructionVisitor
	{
	private:
	    bool changed;
	    Module *module;
	    AccessMap accessMap;
	    CheckCalls checkCalls;
	    BoundsCheckCodeGenerator *codeGen;
	
	public:
	    BoundsCheckVisitor(BoundsCheckCodeGenerator *codeGen);
	    bool insertedChecks();
	    virtual void visitGetElementPtrInst(GetElementPtrInst *inst);
	    AccessMap getArrayAccessMap();
	    CheckCalls getAllCheckCalls();
	    BoundsCheckCodeGenerator *getCodeGenerator();
	};
}

#endif
