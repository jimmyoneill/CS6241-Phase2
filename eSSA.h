#ifndef ESSA_H
#define ESSA_H

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
	std::map<Instruction*, std::map<std::string, int> > var_map;
	std::map<std::string, int> static_var_map;
	std::vector<std::string> names;	

	/*
	(pred block) -> (succ block) -> (eSSAedge*)
	*/
	std::map<BasicBlock*, std::map<BasicBlock*, eSSAedge*> > edges;


	/*
	(branch inst) -> (cmp inst)
	- for the branches that create eSSAedges, the compares that determine the branch
	*/
	std::map<BranchInst*, CmpInst*> br_to_cmp;

	/*
	(check instruction) -> (piAssignment)	
	*/
	std::map<Instruction*, piAssignment*> check_pi_assignments;

	std::vector<CallInst*> call_insts_removed;
	
	eSSA(Module &m, std::string check_func_name);
	void rename(DominatorTree &DT);
	void dom_tree_preorder(DomTreeNode *curr_node);
	void clear_static_var_map(); 
	void rename_var_from_node(std::string operandName, DomTreeNode *curr_node, int newSub);
	void print_var_map(DomTreeNode *curr_node);
	std::vector<GraphConstruct::CGGraph*> find_constraints(Module &m, nbci::NaiveBoundsCheckInserter& inputNBCI); //called from phase1CDpass
	void output_test(Module &m);
	std::string get_mapped_name(std::string name, Instruction *inst);
	std::string int_to_string(int i);



private:
	
	std::string check_func_name;
	void SSA_to_eSSA(Module &m);
	void init_var_map(Module &m);
	void init_pi_assignments(Module &m);
	void handle_br_pi_assignment(BranchInst *branchInst, BasicBlock *bb);  
	void handle_check_pi_assignment(CallInst *inst); 
	void make_branch_conditionals(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb);
	void make_pi_assignments_for_br(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb);
	void add_name_to_var_map(std::string name); 
	void rename_pi_assignments(DomTreeNode *curr_node); 
	void rename_phi_assignments(DomTreeNode *curr_node); 
	void rename_var_from_inst(std::string operandName, DomTreeNode *curr_node, int newSub, Instruction *instruction);
	Instruction* get_next_instruction(BasicBlock *b, Instruction *i);
	
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
