#include "eSSA.h"
#include <sstream>


eSSA::eSSA(Module &m, std::string check_func_name) {

	D = true;
	this->check_func_name = check_func_name;
	SSA_to_eSSA(m);    	
} 

void eSSA::SSA_to_eSSA(Module &m) {

	init_var_map(m);
	init_pi_assignments(m);

	std::map<BasicBlock*, std::map<BasicBlock*, eSSAedge*> >::iterator it1; //ugly as hell - typedef later
	std::map<BasicBlock*, eSSAedge*>::iterator it2;

	//spit out pi functions
	if (D) {
	for (it1 = edges.begin(); it1 != edges.end(); it1++) {
		for (it2 = edges[it1->first].begin(); it2 != edges[it1->first].end(); it2++) {
			std::vector<piAssignment *> pis = edges[it1->first][it2->first]->piAssignments;


			if (pis.size() > 0) {
				errs() << "pi funcs generated from " << it1->first->getName() 
					<< " to " << it2->first->getName() << "\n";
			}
			for (size_t i = 0; i < pis.size(); i++) {
				errs() << "pi assignment operand name " << pis.at(i)->operandBaseName << "\n";
			}
		}
	}
	}
}

void eSSA::init_var_map(Module &m) {
//init the variable map

	if (D) errs() << "eSSA::init_var_map\n";

	//find all the variable names 
	for (Module::iterator f = m.begin(); f != m.end(); f++) {

		//takes care of function arguments
		//ArgumentListType args = f->getArgumentList();
		for (Function::arg_iterator arg = f->arg_begin(); arg != f->arg_end(); ++arg) {
			if (arg->hasName()) {
				//errs() << "arg has name " << arg->getName() << "\n";
				add_name_to_var_map(arg->getName().str());
			}
		} 

	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
    		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
			if (inst->hasName()) {
				add_name_to_var_map(inst->getName().str());
			}			
		}		
    	    }	    
	}

	//for each instruction, set all the names to zero
	std::vector<std::string>::iterator it;
	for (Module::iterator f = m.begin(); f != m.end(); f++) {
	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
			for (size_t i = 0; i < names.size(); i++) {
				var_map[(Instruction*)inst][names.at(i)] = 0;
			//errs() << names.at(i) << " -> " << var_map[(Instruction*)inst][names.at(i)] << "\n";
			}		
		}		
       	    }	    
	}

	clear_static_var_map();

}

void eSSA::add_name_to_var_map(std::string name) {

	if(std::find(names.begin(), names.end(), name) != names.end()) {
    		// names contains the name already
	} 
	else {
		names.push_back(name);
	}


}

void eSSA::clear_static_var_map() {

	for (size_t i = 0; i < names.size(); i++) {
		static_var_map[names.at(i)] = 0;
	}	


}

void eSSA::init_pi_assignments(Module &m) {
	//find and make pi assignments	
	if (D) errs() << "eSSA::init_var_map\n";

	for (Module::iterator f = m.begin(); f != m.end(); f++) {
	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
    		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {

	
			if (BranchInst *branchInst = dyn_cast<BranchInst>(&*inst)) {
				//make pi assignments/edges that correspond to branch
                		handle_br_pi_assignment(branchInst, b);
            		}

	    		if (CallInst *i = dyn_cast<CallInst>(inst)) {
				//make pi assignments that correspond to checks

				Function* calledFunc = i->getCalledFunction();
				if (calledFunc) {
			    		//calls to external functions outside the compilation unit will be null here
			    		if (calledFunc->getName() == check_func_name) {
						//make pi assignment 
						handle_check_pi_assignment(i);
			    		}
				}			
    		    	}			
		}		
    	    }	    
	}	
}

void eSSA::handle_br_pi_assignment(BranchInst *branchInst, BasicBlock *bb) {

	//prune BB and make pi assignment
    if (branchInst->isConditional()) {

	if (D) errs() << "condition: " << branchInst->getCondition()->getName() << "\n";
	std::string conditionalName = branchInst->getCondition()->getName().str();
	//backtrack and find the compare statement that defines this name
	
	for (BasicBlock::iterator inst = bb->begin(); inst != bb->end(); inst++) {
		if (CmpInst *cmpInst = dyn_cast<CmpInst>(&*inst)) {
			if  (cmpInst->getName().str() == conditionalName) {
				if (cmpInst->isIntPredicate()) {
					CmpInst::Predicate pred = cmpInst->getPredicate();
					if ( (pred >= CmpInst::ICMP_SGT) && (pred <= CmpInst::ICMP_SLE) ) {
						if (D) errs() << "making pi assignment info for condition " << conditionalName << "\n";
						br_to_cmp[branchInst] = cmpInst;
						make_pi_assignments_for_br(branchInst, cmpInst, bb);
					}
				}
			}
            	}
	}
    }	
}

void eSSA::make_pi_assignments_for_br(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb) {

	if (branchInst->getNumSuccessors() < 2) errs() << "Branch without 2 successors in eSSA::make_pi_assignments_for_br\n";
		
	for (size_t i = 0; i < branchInst->getNumSuccessors(); i++) {
		if (D) errs() << "successor: " << branchInst->getSuccessor(i)->getName() << "\n";
		eSSAedge *edge = new eSSAedge();

		for (User::op_iterator op = cmpInst->op_begin(); op != cmpInst->op_end(); op++) {
			if (op->get()->hasName()) {
			    	//errs() << op->get()->getName() << " - ";
				edge->piAssignments.push_back( new piAssignment(op->get()->getName().str()) );
			} 

			// if (ConstantInt *constInt = dyn_cast<ConstantInt>(&*op->get())) {
			//     //this will play a role in contraints, but not in pi assignments
			// }
		}
		//add the eSSAedge instance to the edges member
		edges[bb][branchInst->getSuccessor(i)] = edge;
	}
}

void eSSA::handle_check_pi_assignment(CallInst *inst) {

	std::string name;

	//get name of first argument - the variable to be checked
	name = inst->getArgOperand(0)->getName().str();
	
	if (D) errs() << "making pi assignment for name " << name << "\n";
	if (D) errs() << "at instruction " << *inst << "\n";
	piAssignment *pa = new piAssignment(name);
	check_pi_assignments[inst] = pa;
}

void eSSA::rename(DominatorTree &DT) {

	dom_tree_preorder(DT.getRootNode());
	rename_pi_assignments(DT.getRootNode());
	rename_phi_assignments(DT.getRootNode());

}

void eSSA::dom_tree_preorder(DomTreeNode *curr_node) {
//do a preorder traversal of the dom tree, rename when we find pi assignments

	BasicBlock *BB = curr_node->getBlock();
	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) { 
		
		BasicBlock *PredBB = *PI;
		//check if eSSAedge exists, if it does and contains pi assignments, reorder from that node
		if (edges[PredBB][BB] != NULL) {
			std::vector<piAssignment *> pis = edges[PredBB][BB]->piAssignments;
			if (D) errs() << "from " << PredBB->getName() << " to " << BB->getName() << "\n";
			for (size_t i = 0; i < pis.size(); i++) {  
				if (D) errs() << "pi assignment operand name " << pis.at(i)->operandBaseName << "\n";
				static_var_map[pis.at(i)->operandBaseName] += 1;
				int newSub = static_var_map[pis.at(i)->operandBaseName];
				rename_var_from_node(pis.at(i)->operandBaseName, curr_node, newSub);
			}
		}
	}

	//iterate through instructions in block to find a check with a piAssignment	
	for (BasicBlock::iterator inst = BB->begin(); inst != BB->end(); inst++) {
		if(check_pi_assignments[inst]) {
			piAssignment *pa = check_pi_assignments[inst];
			static_var_map[pa->operandBaseName] += 1;
			int newSub = static_var_map[pa->operandBaseName];
			rename_var_from_inst(pa->operandBaseName, curr_node, newSub, inst);
		}		
	}	

        for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		dom_tree_preorder(*child);
        }
}

void eSSA::rename_var_from_node(std::string operandBaseName, DomTreeNode *curr_node, int newSub) {
//for curr_node and all of the nodes that it dominates, go through and rename the variable operandName with the subscript newSub

	if (D) errs() << "renaming " << operandBaseName << " in block " << curr_node->getBlock()->getName() << "\n";
	//var_map[curr_node->getBlock()][operandName] = newSub;

	BasicBlock *b = curr_node->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		var_map[(Instruction*)inst][operandBaseName] = newSub;
	}		

	for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		rename_var_from_node(operandBaseName, *child, newSub);
	}
}

void eSSA::rename_var_from_inst(std::string operandBaseName, DomTreeNode *curr_node, int newSub, Instruction *instruction) {

	bool instruction_hit = false;
	BasicBlock *b = curr_node->getBlock();

	//go through the remaining instructions in the block and rename those
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {

		if(instruction_hit) {
			var_map[(Instruction*)inst][operandBaseName] = newSub;
		}

		if(instruction == inst) {
			if(D) errs() << "renaming " << operandBaseName << " from instruction:\n" << *inst << "\n";
			instruction_hit = true;
		}
	}

	//then rename all of the blocks that this containing block dominates
	for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		rename_var_from_node(operandBaseName, *child, newSub);
	}	
} 

void eSSA::rename_pi_assignments(DomTreeNode *curr_node) {
	
	if (D) errs() << "rename_pi_assignments\n";

	BasicBlock *BB = curr_node->getBlock();
	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) { 
		
		BasicBlock *PredBB = *PI;

		if (PredBB == NULL || BB == NULL) {
			errs() << "BB is NULL in rename_pi_assignments \n";
		}
		
		if (edges[PredBB][BB] != NULL) {
			std::vector<piAssignment *> pis = edges[PredBB][BB]->piAssignments;
			if (D) errs() << "from " << PredBB->getName() << " to " << BB->getName() << "\n";

			for (size_t i = 0; i < pis.size(); i++) {  
				std::string unmappedName = pis.at(i)->operandBaseName; 
				if (D) errs() << "changed pi assignment operand subscript for name " << pis.at(i)->operandBaseName << "\n";
				//pis.at(i)->operandSubscript = get_mapped_name(pis.at(i)->operandBaseName, PredBB->getTerminator()); 
				pis.at(i)->operandSubscript = var_map[PredBB->getTerminator()][unmappedName]; 	
				if (D) errs() << "to " << pis.at(i)->operandSubscript << "\n";
				//pis.at(i)->assignedName = get_mapped_name(unmappedName, BB->getFirstNonPHI()); 
				pis.at(i)->assignedSubscript = var_map[BB->getFirstNonPHI()][unmappedName]; 
				if (D) errs() << "set pi assignment assigned subscript for name " << unmappedName << "\n";
				if (D) errs() << "to " << pis.at(i)->assignedSubscript << "\n";		
			}									
		}
	}

	for (BasicBlock::iterator inst = BB->begin(); inst != BB->end(); inst++) {
		if(check_pi_assignments[inst]) {
			piAssignment *pa = check_pi_assignments[inst];
			std::string unmappedName = pa->operandBaseName; 
			if (D) errs() << "changed pi assignment subscript for operand name " << pa->operandBaseName << "\n";
			//pa->operandName = get_mapped_name(pa->operandName, inst);
			pa->operandSubscript = var_map[inst][unmappedName]; 	 	
			if (D) errs() << "to " << pa->operandSubscript << "\n";
			if(Instruction *next = get_next_instruction(BB, inst)) {
				//get_mapped_name(unmappedName, next); 
				pa->assignedSubscript = var_map[next][unmappedName]; 
			}
			else errs() << "could not get next instruction in rename_pi_assignments\n";
			if (D) errs() << "set pi assignment assigned name " << unmappedName << "\n";
			if (D) errs() << "to " << pa->getAssignedName() << "\n";	
		}						
	}		
		
        for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		rename_pi_assignments(*child);
        }
}

void eSSA::rename_phi_assignments(DomTreeNode *curr_node) {
//for each phi function, change the var_map at its Instruction* to map the same to the BBs the vars came from 
	BasicBlock *b = curr_node->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {		
		if (PHINode *pn = dyn_cast<PHINode>(inst)) {
			for (size_t i = 0; i < pn->getNumIncomingValues(); ++i) {
				Value *v = pn->getIncomingValue(i);
				if(v->hasName()) {
					BasicBlock *incomingBlock = pn->getIncomingBlock(i);
					int new_sub;
					bool in_essa_edge = false;
					/*
					Check if there is an eSSAedge with piAssignments. This will happen at critical edges
					If there is, make sure that if the piAssignments renamed a variable that is 
					in the current phi, node, use that new assigned name
					*/
					if (edges[incomingBlock][b]) {
						std::vector<piAssignment *> pis = edges[incomingBlock][b]->piAssignments;
						for (size_t p = 0; p < pis.size(); ++p) {
							if(v->getName().str() == pis.at(p)->assignedBaseName) {
								in_essa_edge = true;
								new_sub = pis.at(p)->assignedSubscript;
								var_map[inst][v->getName().str()] = new_sub;
								break;
							}
						}
					}
					/*
					If the var wasn't present in an eSSAedge, set the mapping at the phi node to be the
					same as the last instruction in the pred block.
					*/
					if (!in_essa_edge) {
						new_sub = var_map[pn->getIncomingBlock(i)->getTerminator()][v->getName().str()];
						//associate the subscript with the variable at the phi node
						var_map[inst][v->getName().str()] = new_sub;
					}
				}
			}	
		}	
	}

	for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		rename_pi_assignments(*child);
        }
}

void eSSA::make_branch_conditionals(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb) {
	//this is everything needed to make C4
}

//called from phase1CDpass
std::vector<GraphConstruct::CGGraph*> eSSA::find_constraints(Module &m, nbci::NaiveBoundsCheckInserter& inputNBCI) {
	//find constraints	

	std::vector<GraphConstruct::CGGraph*> toReturn;

	for (Module::iterator f = m.begin(); f != m.end(); f++) {

		if(1) errs() << "\n------------- making CGGraph for function named " << f->getName() << "\n\n";
		GraphConstruct::CGGraph *cggraph = new GraphConstruct::CGGraph(this, f->getName().str());

		cggraph->addArrayLengths(inputNBCI.getBoundsCheckVisitor()->getArrayAccessMap());
	
	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
    		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
				
			//CONTROL FLOW
			if (PHINode *phiNode = dyn_cast<PHINode>(inst)) {
				if (phiNode->getNumIncomingValues() == 2) {
						if (phiNode->getIncomingValue(0)->getType()->isIntegerTy() && phiNode->getIncomingValue(1)->getType()->isIntegerTy()) {
						if (D) errs() << "got CONTROL FLOW \n";
						if (D) errs() << "\n";
						cggraph->addConstraint(inst);
					}
				}
			}	
			//C1 and C2
			else if (StoreInst* storeInst = dyn_cast<StoreInst>(inst)) {
				if (storeInst->getValueOperand()->getType()->isIntegerTy()) {
					if (storeInst->getPointerOperand()->hasName()) {
						if (D) errs() << "inst: " << *inst << "\n"
						<< "puts value " << storeInst->getValueOperand()->getName() << " into " 
						<< storeInst->getPointerOperand()->getName() << "\n";
						if (D) errs() << "GOT C1/C2 \n";
						if (D) errs() << "\n";
						cggraph->addConstraint(inst);

					}
				}
			}
			else if (LoadInst* loadInst = dyn_cast<LoadInst>(inst)) {
				if (loadInst->getPointerOperand()->hasName()) {
					if (D) errs() << "inst: " << *inst << "\n"
					<< "loads " << loadInst->getPointerOperand()->getName() << " into " << loadInst->getName() << "\n";
					if (D) errs() << "contained type is integer type?: " 
					<< loadInst->getPointerOperand()->getType()->getContainedType(0)->isIntegerTy() << "\n";
					if (D) errs() << "GOT C1/C2 \n";
					if (D) errs() << "\n";

					//if integer is being loaded, send it
					if (loadInst->getPointerOperand()->getType()->getContainedType(0)->isIntegerTy()) { 
						cggraph->addConstraint(inst);
					}
				}
			}
			else if (CastInst* castInst = dyn_cast<CastInst>(inst)) {
				if (castInst->isIntegerCast()) {
					if (castInst->hasName()) {
						if (D) errs() << "GOT INT CAST\n";
						if (D) errs() << "inst: " << *inst << "\n"
						<< "loads " << castInst->getOperand(0)->getName() << " into " << castInst->getName() << "\n";
						if (D) errs() << "GOT C1/C2 \n";
						if (D) errs() << "\n";
						cggraph->addConstraint(inst);
					}
				}
			}	
			//C3 - has only one variable and one int
			else if (isa<BinaryOperator>(inst)) {	    	
				int intCount = 0;
				int varCount = 0;
				for (User::op_iterator op = inst->op_begin(); op != inst->op_end(); op++) {
					if (op->get()->hasName()) ++varCount;
					else if (isa<ConstantInt>(op->get())) ++intCount;
				}
				
				if ( (intCount == 1) && (varCount == 1) ) {
					if (D) errs() << "got C3 \n";
					if (D) errs() << "\n";		
					cggraph->addConstraint(inst);
				}
			}
			//C4 - is a branch and corresponds to pi assignments			
			else if (BranchInst *branchInst = dyn_cast<BranchInst>(&*inst)) {
				
				if (branchInst->getNumSuccessors() == 2) {
					bool acceptConstraint = true;
					std::vector<piAssignment *> pis1;
					std::vector<piAssignment *> pis2;
					for (size_t i = 0; i < branchInst->getNumSuccessors(); i++) {
						BasicBlock* pred = branchInst->getParent();
						BasicBlock* succ = branchInst->getSuccessor(i);					
						if (edges[pred][succ]) {
							std::vector<piAssignment *> pis;
							pis = edges[pred][succ]->piAssignments;
							if (pis.size() == 0) {
								errs() << "ERROR: pis has no assignments in eSSA::find_constraints \n";
								acceptConstraint = false;
								break;
							}
							if (i == 0) pis1 = pis;
							else pis2 = pis;
						}
						else {
							acceptConstraint = false;
							break;
						}
					}
					if (acceptConstraint) {
						if (D) errs() << "got C4:\n";
						if (D) errs() << *branchInst << "\n";
						if (D) errs() << "\n";
						cggraph->addConstraint(branchInst, pis1, pis2);
					}
				}				
            		}
			//C5 - is the a check function call
			else if (CallInst *i = dyn_cast<CallInst>(inst)) {
				//make pi assignments that correspond to checks
				if(check_pi_assignments[inst]) {
					if (D) errs() << "GOT C5\n";
					if (D) errs() << "\n";
					piAssignment *pa = check_pi_assignments[inst];
					cggraph->addConstraint(i, pa);
				}

    		    	}									
		} // end BasicBlock::iterator		
    	    } // end Function::iterator 

		cggraph->constructGraph();	

		toReturn.push_back(cggraph);

	} // end Module::iterator

	return toReturn;

} // end find_constraints


std::string eSSA::get_mapped_name(std::string name, Instruction *inst) {

	//tag on the eSSA subscript to make a string literal
	int subscript = var_map[inst][name]; 
	return name + int_to_string(subscript);
}


std::string eSSA::int_to_string(int i) {

	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
}

void eSSA::print_var_map(DomTreeNode *curr_node) {

	BasicBlock* b = curr_node->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		for (size_t i = 0; i < names.size(); i++) {
			errs() << b->getName() << " - " << names.at(i) << " - " << var_map[(Instruction*)inst][names.at(i)] << "\n";
		}	
	}		

	for(DomTreeNode::iterator child = curr_node->begin(); child != curr_node->end(); ++child) {
		print_var_map(*child);
	}
}

void eSSA::output_test(Module &m) {

	errs() << "\noutput test: \n";
	for (Module::iterator f = m.begin(); f != m.end(); f++) {
		errs() << "\nFUNCTION: " << f->getName() << "\n";
		for (Function::iterator b = f->begin(); b != f->end(); b++) {
			errs() << "\nBLOCK: " << b->getName() << "\n";
    			for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
				errs() << *inst << "\n";
				errs() << "mapped vars  ";
				for (User::op_iterator op = inst->op_begin(); op != inst->op_end(); op++) {
					if (op->get()->hasName()) {
						std::string name = op->get()->getName(); 
			    			errs() << " - " << get_mapped_name(name, inst);
					} 
				}
				errs() << "\n";	
				if(check_pi_assignments[inst]) {
					piAssignment *pa = check_pi_assignments[inst];
					errs() << "PI ASSIGNMENT: " << pa->getAssignedName() << " = " << pa->getOperandName() << "\n";
				}

				if (BranchInst *branchInst = dyn_cast<BranchInst>(&*inst)) {

					for (size_t i = 0; i < branchInst->getNumSuccessors(); i++) {
						BasicBlock* pred = branchInst->getParent();
						BasicBlock* succ = branchInst->getSuccessor(i);						
						if (edges[pred][succ]) {
							errs() << "PI ASSIGNMENT(S) ON EDGE TO: " 
							<< branchInst->getSuccessor(i)->getName() << "\n";

							std::vector<piAssignment *> pis;
							pis = edges[pred][succ]->piAssignments;
							for (size_t i = 0; i< pis.size(); ++i) {
								piAssignment *pa = pis.at(i);
								errs() << "PI ASSIGNMENT: " << pa->getAssignedName() 
								<< " = " << pa->getOperandName() << "\n";
							}
						}
					}
				}		
			}	
    		}	    
	}	
}

Instruction* eSSA::get_next_instruction(BasicBlock *b, Instruction *i) {

	bool instruction_hit = false;
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		if (instruction_hit) return inst;
		if ((Instruction*)inst == i) instruction_hit = true;				
	}
	return NULL;		
}



