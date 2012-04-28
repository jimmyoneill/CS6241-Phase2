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
			AU.addRequired<DominatorTree>();
			AU.addRequired<NaiveBoundsCheckInserter>();
		}

		virtual bool runOnModule(Module &m) {

			NaiveBoundsCheckInserter &nbci = getAnalysis<NaiveBoundsCheckInserter>();
			eSSA *essa = new eSSA(m, nbci.getCheckFuncName());

			for (Module::iterator f = m.begin(); f != m.end(); f++) {
				//rename variables in eSSA
				if (!(f->empty())) {
					essa->rename(getAnalysis<DominatorTree>(*f));
				}

			}

			std::vector<GraphConstruct::CGGraph*> graphs = essa->find_constraints(m, nbci);
			std::vector<GraphConstruct::CGGraph*>::iterator graphIt;
			GraphConstruct::CGGraph* graph;

			for (graphIt = graphs.begin(); graphIt != graphs.end(); ++graphIt) {

				graph = *graphIt;

				(*graphIt)->solve(nbci.getBoundsCheckVisitor()->getAllCheckCalls());

			}
			
			return false;

		}	

	};

}
  
char p1CDPass::ID = 0;
static RegisterPass<p1CDPass> Y("phase1CDPass", "Phase 1 Constraint Detection Pass", false, false);

