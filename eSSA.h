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
class eSSAedge;
class piAssignment;

class eSSA {

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
    (pred block) -> (succ block) -> (eSSAedge*)
    */
    std::map<BasicBlock*, std::map<BasicBlock*, eSSAedge*> > edges;


    /*
    (branch inst) -> (cmp inst)
    - for the branches that create eSSAedges, the compares that determine the branch
    */
    std::map<BranchInst*, CmpInst*> branchToCompare;

    /*
    (check instruction) -> (piAssignment)	
    */
    std::map<Instruction*, piAssignment*> checkPiAssignments;
    std::vector<CallInst*> callInstsRemoved;
    
    eSSA(Module &m, std::string checkFuncName);
    void rename(DominatorTree &DT);
    void renameVarFromNode(std::string operandName, DomTreeNode *curr_node, int newSub);
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
    void rename_pi_assignments(DomTreeNode *curr_node); 
    void rename_phi_assignments(DomTreeNode *curr_node); 
    void rename_var_from_inst(std::string operandName, DomTreeNode *curr_node, int newSub, Instruction *instruction);
    Instruction* get_next_instruction(BasicBlock *b, Instruction *i);
    void print_pi_functions();
    std::string int_to_string(int i);
    void print_var_map(DomTreeNode *curr_node);
    void dom_tree_preorder(DomTreeNode *curr_node);
	void clear_static_var_map(); 
	
};

class eSSAedge {

	public:
	//this will hold the pi assignments in edges
	std::vector<piAssignment *> piAssignments;
	
	eSSAedge() {
		//D = false;
	};

};


#endif
