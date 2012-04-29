#include "eSSA.h"
#include <sstream>


eSSA::eSSA(Module &m, std::string checkFuncName) {

	D = false;
	this->checkFuncName = checkFuncName;
	SsaToEssa(m);    	
} 

void eSSA::SsaToEssa(Module &m) {

	initVarMap(m);
	initPiAssignments(m);
    if (D) printPiFunctions();
}

void eSSA::initVarMap(Module &m) {
//init the variable map

	if (D) errs() << "eSSA::initVarMap\n";

	//find all the variable names 
	for (Module::iterator f = m.begin(); f != m.end(); f++) {

		//takes care of function arguments
		//ArgumentListType args = f->getArgumentList();
		for (Function::arg_iterator arg = f->arg_begin(); arg != f->arg_end(); ++arg) {
			if (arg->hasName()) {
				//errs() << "arg has name " << arg->getName() << "\n";
				addNameToVarMap(arg->getName().str());
			}
		} 

	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
    		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
                if (inst->hasName()) {
                    addNameToVarMap(inst->getName().str());
                }			
            }		
        }	    
    }

	clearStaticVarMap();
}

void eSSA::addNameToVarMap(std::string name) {

	if(std::find(names.begin(), names.end(), name) != names.end()) {
    		// names contains the name already
	} 
	else {
		names.push_back(name);
	}
}

void eSSA::clearStaticVarMap() {

	for (size_t i = 0; i < names.size(); i++) {
		staticVarMap[names.at(i)] = 0;
	}	
}

void eSSA::initPiAssignments(Module &m) {
	//find and make pi assignments	
	if (D) errs() << "eSSA::initVarMap\n";

	for (Module::iterator f = m.begin(); f != m.end(); f++) {
	    for (Function::iterator b = f->begin(); b != f->end(); b++) {
    		for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
                if (BranchInst *branchInst = dyn_cast<BranchInst>(&*inst)) {
                    //make pi assignments/edges that correspond to branch
                    handleBranchAtPiAssignment(branchInst, b);
                }

                if (CallInst *i = dyn_cast<CallInst>(inst)) {
                    //make pi assignments that correspond to checks
                    Function* calledFunc = i->getCalledFunction();
                    if (calledFunc) {
                        //calls to external functions outside the compilation unit will be null here
                        if (calledFunc->getName() == checkFuncName) {
                            //make pi assignment 
                            handleCheckAtPiAssignment(i);
                        }
                    }			
                }			
            }		
        }	    
    }	
}

void eSSA::handleBranchAtPiAssignment(BranchInst *branchInst, BasicBlock *bb) {

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
                            branchToCompare[branchInst] = cmpInst;
                            makePiAssignmentsForBranch(branchInst, cmpInst, bb);
                        }
                    }
                }
            }
        }
    }	
}

void eSSA::makePiAssignmentsForBranch(BranchInst *branchInst, CmpInst *cmpInst, BasicBlock *bb) {

	if (branchInst->getNumSuccessors() < 2) errs() << "Branch without 2 successors in eSSA::makePiAssignmentsForBranch\n";
		
	for (size_t i = 0; i < branchInst->getNumSuccessors(); i++) {
		if (D) errs() << "successor: " << branchInst->getSuccessor(i)->getName() << "\n";
		eSSAedge *edge = new eSSAedge();

		for (User::op_iterator op = cmpInst->op_begin(); op != cmpInst->op_end(); op++) {
			if (op->get()->hasName()) {
                if (D) errs() << op->get()->getName() << " - ";
				edge->piAssignments.push_back( new piAssignment(op->get()->getName().str()) );
			} 
		}
		//add the eSSAedge instance to the edges member
		edges[bb][branchInst->getSuccessor(i)] = edge;
	}
}

void eSSA::handleCheckAtPiAssignment(CallInst *inst) {

	std::string name;

	//get name of first argument - the variable to be checked
	name = inst->getArgOperand(0)->getName().str();
	
	if (D) errs() << "making pi assignment for name " << name << "\n";
	if (D) errs() << "at instruction " << *inst << "\n";
	piAssignment *pa = new piAssignment(name);
	checkPiAssignments[inst] = pa;
}

void eSSA::rename(DominatorTree &DT) {

	domTreePreorder(DT.getRootNode());
	renamePiAssignments(DT.getRootNode());
	renamePhiAssignments(DT.getRootNode());
}

void eSSA::domTreePreorder(DomTreeNode *currNode) {
//do a preorder traversal of the dom tree, rename when we find pi assignments

	BasicBlock *BB = currNode->getBlock();
	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) { 
		
		BasicBlock *PredBB = *PI;
		//check if eSSAedge exists, if it does and contains pi assignments, reorder from that node
		if (edges[PredBB][BB] != NULL) {
			std::vector<piAssignment *> pis = edges[PredBB][BB]->piAssignments;
			if (D) errs() << "from " << PredBB->getName() << " to " << BB->getName() << "\n";
			for (size_t i = 0; i < pis.size(); i++) {  
				if (D) errs() << "pi assignment operand name " << pis.at(i)->operandBaseName << "\n";
				staticVarMap[pis.at(i)->operandBaseName] += 1;
				int newSub = staticVarMap[pis.at(i)->operandBaseName];
				renameVarFromNode(pis.at(i)->operandBaseName, currNode, newSub);
			}
		}
	}

	//iterate through instructions in block to find a check with a piAssignment	
	for (BasicBlock::iterator inst = BB->begin(); inst != BB->end(); inst++) {
		if(checkPiAssignments[inst]) {
			piAssignment *pa = checkPiAssignments[inst];
			staticVarMap[pa->operandBaseName] += 1;
			int newSub = staticVarMap[pa->operandBaseName];
			renameVarFromInst(pa->operandBaseName, currNode, newSub, inst);
		}		
	}	

    for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		domTreePreorder(*child);
    }
}

void eSSA::renameVarFromNode(std::string operandBaseName, DomTreeNode *currNode, int newSub) {
//for currNode and all of the nodes that it dominates, go through and rename the variable operandName with the subscript newSub

	if (D) errs() << "renaming " << operandBaseName << " in block " << currNode->getBlock()->getName() << "\n";
	//varMap[currNode->getBlock()][operandName] = newSub;

	BasicBlock *b = currNode->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		varMap[(Instruction*)inst][operandBaseName] = newSub;
	}		

	for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		renameVarFromNode(operandBaseName, *child, newSub);
	}
}

void eSSA::renameVarFromInst(std::string operandBaseName, DomTreeNode *currNode, int newSub, Instruction *instruction) {

	bool hitInstruction = false;
	BasicBlock *b = currNode->getBlock();

	//go through the remaining instructions in the block and rename those
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {

		if(hitInstruction) {
			varMap[(Instruction*)inst][operandBaseName] = newSub;
		}

		if(instruction == inst) {
			if(D) errs() << "renaming " << operandBaseName << " from instruction:\n" << *inst << "\n";
			hitInstruction = true;
		}
	}

	//then rename all of the blocks that this containing block dominates
	for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		renameVarFromNode(operandBaseName, *child, newSub);
	}	
} 

void eSSA::renamePiAssignments(DomTreeNode *currNode) {
	
	if (D) errs() << "renamePiAssignments\n";

	BasicBlock *BB = currNode->getBlock();
	for (pred_iterator PI = pred_begin(BB), E = pred_end(BB); PI != E; ++PI) { 
		
		BasicBlock *PredBB = *PI;

		if (PredBB == NULL || BB == NULL) {
			errs() << "BB is NULL in renamePiAssignments \n";
		}
		
		if (edges[PredBB][BB] != NULL) {
			std::vector<piAssignment *> pis = edges[PredBB][BB]->piAssignments;
			if (D) errs() << "from " << PredBB->getName() << " to " << BB->getName() << "\n";

			for (size_t i = 0; i < pis.size(); i++) {  
				std::string unmappedName = pis.at(i)->operandBaseName; 
				if (D) errs() << "changed pi assignment operand subscript for name " << pis.at(i)->operandBaseName << "\n";
				//pis.at(i)->operandSubscript = getMappedName(pis.at(i)->operandBaseName, PredBB->getTerminator()); 
				pis.at(i)->operandSubscript = varMap[PredBB->getTerminator()][unmappedName]; 	
				if (D) errs() << "to " << pis.at(i)->operandSubscript << "\n";
				//pis.at(i)->assignedName = getMappedName(unmappedName, BB->getFirstNonPHI()); 
				pis.at(i)->assignedSubscript = varMap[BB->getFirstNonPHI()][unmappedName]; 
				if (D) errs() << "set pi assignment assigned subscript for name " << unmappedName << "\n";
				if (D) errs() << "to " << pis.at(i)->assignedSubscript << "\n";		
			}									
		}
	}

	for (BasicBlock::iterator inst = BB->begin(); inst != BB->end(); inst++) {
		if(checkPiAssignments[inst]) {
			piAssignment *pa = checkPiAssignments[inst];
			std::string unmappedName = pa->operandBaseName; 
			if (D) errs() << "changed pi assignment subscript for operand name " << pa->operandBaseName << "\n";
			//pa->operandName = getMappedName(pa->operandName, inst);
			pa->operandSubscript = varMap[inst][unmappedName]; 	 	
			if (D) errs() << "to " << pa->operandSubscript << "\n";
			if(Instruction *next = getNextInstruction(BB, inst)) {
				//getMappedName(unmappedName, next); 
				pa->assignedSubscript = varMap[next][unmappedName]; 
			}
			else errs() << "could not get next instruction in renamePiAssignments\n";
			if (D) errs() << "set pi assignment assigned name " << unmappedName << "\n";
			if (D) errs() << "to " << pa->getAssignedName() << "\n";	
		}						
	}		
		
    for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		renamePiAssignments(*child);
    }
}

void eSSA::renamePhiAssignments(DomTreeNode *currNode) {
//for each phi function, change the varMap at its Instruction* to map the same to the BBs the vars came from 
	BasicBlock *b = currNode->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {		
		if (PHINode *pn = dyn_cast<PHINode>(inst)) {
			for (size_t i = 0; i < pn->getNumIncomingValues(); ++i) {
				Value *v = pn->getIncomingValue(i);
				if(v->hasName()) {
					BasicBlock *incomingBlock = pn->getIncomingBlock(i);
					int newSub;
					bool isInEssaEdge = false;
					/*
					Check if there is an eSSAedge with piAssignments. This will happen at critical edges
					If there is, make sure that if the piAssignments renamed a variable that is 
					in the current phi, node, use that new assigned name
					*/
					if (edges[incomingBlock][b]) {
						std::vector<piAssignment *> pis = edges[incomingBlock][b]->piAssignments;
						for (size_t p = 0; p < pis.size(); ++p) {
							if(v->getName().str() == pis.at(p)->assignedBaseName) {
								isInEssaEdge = true;
								newSub = pis.at(p)->assignedSubscript;
								varMap[inst][v->getName().str()] = newSub;
								break;
							}
						}
					}
					/*
					If the var wasn't present in an eSSAedge, set the mapping at the phi node to be the
					same as the last instruction in the pred block.
					*/
					if (!isInEssaEdge) {
						newSub = varMap[pn->getIncomingBlock(i)->getTerminator()][v->getName().str()];
						//associate the subscript with the variable at the phi node
						varMap[inst][v->getName().str()] = newSub;
					}
				}
			}	
		}	
	}

	for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		renamePiAssignments(*child);
    }
}

//called from phase1CDpass
std::vector<GraphConstruct::CGGraph*> eSSA::findConstraints(Module &m, nbci::NaiveBoundsCheckInserter& inputNBCI) {
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
                                    errs() << "ERROR: pis has no assignments in eSSA::findConstraints \n";
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
                    if(checkPiAssignments[inst]) {
                        if (D) errs() << "GOT C5\n";
                        if (D) errs() << "\n";
                        piAssignment *pa = checkPiAssignments[inst];
                        cggraph->addConstraint(i, pa);
                    }
                }									
            } // end BasicBlock::iterator		
        } // end Function::iterator 

		cggraph->constructGraph();	
		toReturn.push_back(cggraph);

	} // end Module::iterator

	return toReturn;

} // end findConstraints


std::string eSSA::getMappedName(std::string name, Instruction *inst) {

	//tag on the eSSA subscript to make a string literal
	int subscript = varMap[inst][name]; 
	return name + intToString(subscript);
}


std::string eSSA::intToString(int i) {

	std::string s;
	std::stringstream out;
	out << i;
	s = out.str();
	return s;
}

void eSSA::printVarMap(DomTreeNode *currNode) {

	BasicBlock* b = currNode->getBlock();
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		for (size_t i = 0; i < names.size(); i++) {
			errs() << b->getName() << " - " << names.at(i) << " - " << varMap[(Instruction*)inst][names.at(i)] << "\n";
		}	
	}		

	for(DomTreeNode::iterator child = currNode->begin(); child != currNode->end(); ++child) {
		printVarMap(*child);
	}
}

void eSSA::printPiFunctions() {
    
    std::map<BasicBlock*, std::map<BasicBlock*, eSSAedge*> >::iterator it1; 
	std::map<BasicBlock*, eSSAedge*>::iterator it2;
    
    //spit out pi functions
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

void eSSA::outputTest(Module &m) {

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
                            errs() << " - " << getMappedName(name, inst);
                    } 
                }
                errs() << "\n";	
                    
                if(checkPiAssignments[inst]) {
                    piAssignment *pa = checkPiAssignments[inst];
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

Instruction* eSSA::getNextInstruction(BasicBlock *b, Instruction *i) {

	bool hitInstruction = false;
	for (BasicBlock::iterator inst = b->begin(); inst != b->end(); inst++) {
		if (hitInstruction) return inst;
		if ((Instruction*)inst == i) hitInstruction = true;				
	}
	return NULL;		
}



