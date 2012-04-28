#include "NaiveBoundsCheckInserter.h"

using namespace llvm;
using namespace patterns;
using namespace codegen;
using namespace std;

namespace nbci {

	NaiveBoundsCheckInserter::NaiveBoundsCheckInserter() : ModulePass(ID) {}

	bool NaiveBoundsCheckInserter::runOnModule(Module &M) 
	{

		// Create the code gen.
		BoundsCheckCodeGenerator *codeGen = new BoundsCheckCodeGenerator(&M);
		codeGen->insertFuncDefinitions();

		// Create the visitor
		BoundsCheckVisitor *visitor = new BoundsCheckVisitor(codeGen);

		for(Module::iterator it = M.begin(); it != M.end(); it++)
		{
			Function *func = (Function*)it;
			if(!(func->empty() || func->isDeclaration()))
			{
				func->viewCFG();
				visitor->visit(func);
				func->viewCFG();
			}
		}

		// Store the visitor in case of multiple modules.
		this->visitor = visitor;

		// Need to cast to a module due to overloads
		return visitor->insertedChecks();

	}

	void NaiveBoundsCheckInserter::getAnalysisUsage(AnalysisUsage &AU) const 
	{
		AU.setPreservesCFG();
	}

	void NaiveBoundsCheckInserter::print(raw_ostream &O, const Module *M) const 
	{
		O << "============================= AccessMap =============================\n";
		printArrayAccessMap(O);
		O << "=============================== Checks ==============================\n";
		printChecks(O);
		O << "=====================================================================\n";
	}

	void NaiveBoundsCheckInserter::printArrayAccessMap(raw_ostream &O) const {
		AccessMap accessMap = this->visitor->getArrayAccessMap();

		int i = 1;
		for(AccessMap::iterator it=accessMap.begin(); it != accessMap.end(); it++)
		{
			O << i++ << ":\t";
			it->second->dump(O);
		}
	}

	void NaiveBoundsCheckInserter::printChecks(raw_ostream &O) const {
		CheckCalls calls = this->visitor->getAllCheckCalls();

		int i = 1;
		for(CheckCalls::iterator it = calls.begin(); it != calls.end(); it++)
		{
			O << i++ << ":\t";
			(*it)->dump();
		}
	}

	// Use this to access the visitor for this module. Everything useful lives there.
	BoundsCheckVisitor* NaiveBoundsCheckInserter::getBoundsCheckVisitor() const
	{
		return this->visitor;
	}

	string NaiveBoundsCheckInserter::getCheckFuncName() const
	{
		return this->visitor->getCodeGenerator()->getCheckFuncName();
	}

}

char nbci::NaiveBoundsCheckInserter::ID = 0;
static RegisterPass<nbci::NaiveBoundsCheckInserter> X(
	"nbci", 
	"CS6241 phase 2 naive bounds check insertion pass", 
	false, 
	false);
