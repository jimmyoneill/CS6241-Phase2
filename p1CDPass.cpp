#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CallSite.h" // CallInst
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Intrinsics.h"
#include "llvm/Analysis/Dominators.h"
#include <limits>
#include <map>
#include "eSSA.h"
#include "GraphComponents.h"
#include "NaiveBoundsCheckInserter.h"

using namespace llvm;
using namespace nbci;

namespace {

struct p1CDPass : public ModulePass {

    static char ID;
 
    p1CDPass() : ModulePass(ID) {

    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
		//AU.setPreservesCFG();
		AU.addRequired<DominatorTree>();
		AU.addRequired<NaiveBoundsCheckInserter>();
    }
	
	void graphTest(Module *m) {
		
		//Get a basic block from module
		Module::iterator mi = m->begin(); //Function
		Function::iterator fi = mi->begin(); //Basic block
		fi++;
		BasicBlock::iterator bi = fi->begin(); //Instruction
		
	}

    virtual bool runOnModule(Module &m) {

	//errs() << m;

	NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();
	std::string name = nbci.getCheckFuncName(); 
	errs() << "func name is " << name << "\n";
	
	eSSA *essa = new eSSA(m, nbci.getCheckFuncName());
	bool D = essa->D;
	graphTest(&m);

	for (Module::iterator f = m.begin(); f != m.end(); f++) {
		//rename variables in eSSA
		if (!(f->empty())) {
			if (D) errs () << "\nrenaming in " << f->getName() << "\n";
			essa->rename(getAnalysis<DominatorTree>(*f));											       				 //if (D) essa->print_var_map(getAnalysis<DominatorTree>(*f).getRootNode());
			//if (D) errs() << "\n";
		}

 	}

	errs() << "find constraints \n";
	std::vector<GraphConstruct::CGGraph*> graphs = essa->find_constraints(m, nbci);
	//essa->output_test(m);
        std::vector<GraphConstruct::CGGraph*>::iterator graphIt;
	GraphConstruct::CGGraph* graph;

	for (graphIt = graphs.begin(); graphIt != graphs.end(); ++graphIt) {

		errs() << "In graphIt loop\n";

		graph = *graphIt;

		(*graphIt)->solve(nbci.getBoundsCheckVisitor()->getAllCheckCalls());

	}

	Module::iterator mIt;
	
	for (mIt = m.begin(); mIt != m.end(); ++mIt) { mIt->viewCFG(); }

	return false;

    }	

};

}
  
char p1CDPass::ID = 0;
static RegisterPass<p1CDPass> Y("phase1CDPass", "Phase 1 Constraint Detection Pass", false, false);

