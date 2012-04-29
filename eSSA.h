#ifndef ESSA_H
#define ESSA_H

#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/Instruction.h"
#include "llvm/Pass.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/CallSite.h" 
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Intrinsics.h"
#include "llvm/Analysis/Dominators.h"
#include "llvm/Value.h"
#include "llvm/User.h"
#include "llvm/Use.h"
#include "llvm/Constants.h"
#include "llvm/ADT/Twine.h"
#include <limits>
#include "llvm/Support/CFG.h"
#include "llvm/InstrTypes.h"
#include "GraphComponents.h"
#include <map>
#include "piAssignment.h"
#include "NaiveBoundsCheckInserter.h"

using namespace llvm;
class ESSAedge;
class piAssignment;

class ESSA {

public:

    bool D;

    /*
    (BB*) -> (variable name) -> (rename stack depth)
    - MUST BE RESET FOR EACH FUNCTION BECAUSE OF VARIABLE NAMESPACES
    */
    std::map<Instruction*, std::map<std::string, int> > varMap;
    std::map<std::string, int> staticVarMap;
    std::vector<std::string> names;	

    /*
    (pred block) -> (succ block) -> (ESSAedge*)
    */
    std::map<BasicBlock*, std::map<BasicBlock*, ESSAedge*> > edges;


    /*
    (branch inst) -> (cmp inst)
    - for the branches that create ESSAedges, the compares that determine the branch
    */
    std::map<BranchInst*, CmpInst*> branchToCompare;

    /*
    (check instruction) -> (piAssignment)	
    */
    std::map<Instruction*, piAssignment*> checkPiAssignments;
    std::vector<CallInst*> callInstsRemoved;
    
    ESSA(Module &m, std::string checkFuncName);
    void rename(DominatorTree &DT);
    void renameVarFromNode(std::string operandName, DomTreeNode *currNode, int newSub);
    std::vector<GraphConstruct::CGGraph*> findConstraints(Module &m, nbci::NaiveBoundsCheckInserter& inputNBCI); //called from phase1CDpass
    void outputTest(Module &m);
    std::string getMappedName(std::string name, Instruction *inst);
	
    
private:
	
    std::string checkFuncName;
    void SsaToEssa(Module &m);
    void initVarMap(Module &m);
    void initPiAssignments(Module &m);
    void handleBranchAtPiAssignment(BranchInst *branchInst, BasicBlock *bb);  
    void handleCheckAtPiAssignment(CallInst *inst); 
    void makePiAssignmentsForBranch(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb);
    void addNameToVarMap(std::string name); 
    void renamePiAssignments(DomTreeNode *currNode); 
    void renamePhiAssignments(DomTreeNode *currNode); 
    void renameVarFromInst(std::string operandName, DomTreeNode *currNode, int newSub, Instruction *instruction);
    Instruction* getNextInstruction(BasicBlock *b, Instruction *i);
    void printPiFunctions();
    std::string intToString(int i);
    void printVarMap(DomTreeNode *currNode);
    void domTreePreorder(DomTreeNode *currNode);
	void clearStaticVarMap(); 
	
};

class ESSAedge {

	public:
	//this will hold the pi assignments in edges
	std::vector<piAssignment *> piAssignments;
	
	ESSAedge() {
		//D = false;
	};

};


#endif
