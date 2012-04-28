
#include "GraphComponents.h"
//#include "piAssignment.h"
#include "eSSA.h"

using namespace GraphConstruct;
using namespace llvm;

	/**
	* CGConstraint definitions
	*/

	CGConstraint::CGConstraint(ConType inputType, Instruction* inputInstr, int inputLength) {

		arrayLength = inputLength;
		type = inputType;
		programPoint = inputInstr;

	}

	CGConstraint::CGConstraint(ConType inputType, Instruction* inputInstr) {
	
		type = inputType;
		programPoint = inputInstr;
	
	}
	
	CGConstraint::CGConstraint(ConType inputType, Instruction* inputInstr, std::vector<piAssignment*> inputPi) {
		//C5
		type = inputType;
		programPoint = inputInstr;
		piAssignments = inputPi;
	
	}

	CGConstraint::CGConstraint(ConType inputType, Instruction* inputInstr, std::vector<piAssignment*> inputPi1, std::vector<piAssignment*> inputPi2) {
		//C4
		type = inputType;
		programPoint = inputInstr;
		piAssignments = inputPi1;
		piAssignments2 = inputPi2;
	
	}

	std::string CGConstraint::getDescription(bool doPadding) {

		std::stringstream toReturn;

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "CGConstraint with address of: " << this << "\n";
		toReturn << "CGConstraint type is " << type << "\n";
		toReturn << "Containing basic block is " << containerBlock << "\n";
		toReturn << "Relevant instruction is " << programPoint << "\n\n";

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "\n";

		return toReturn.str();
		
	}

	/**
	* CGNode definitions
	*/

	CGNode::CGNode(CGNodeContent* inputConst) {
		
		content = inputConst;

	}

	void CGNode::connectTo(CGNode* toConnect, int edgeValue) {

		CGEdge* newEdge = new CGEdge(this, toConnect, edgeValue);
		toEdges.push_back(newEdge);
		toConnect->fromEdges.push_back(newEdge);

	}

	void CGNode::connectFrom(CGNode* fromConnect, int edgeValue) {

		CGEdge* newEdge = new CGEdge(fromConnect, this, edgeValue);
		fromEdges.push_back(newEdge);
		fromConnect->toEdges.push_back(newEdge);

	}

	std::string CGNode::getDescription(bool doPadding) {

		std::stringstream toReturn;

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "CGNode with address of: " << this << "\n";
		toReturn << "Constraint contained is at address: " << content << "\n\n";
		
		toReturn << "There are " << fromEdges.size() << " incoming edges pointing to this CGNode\n";
		
		std::vector<CGEdge*>::iterator itr;
		int edgeCounter = 0;
		
		for (itr = fromEdges.begin(); itr != fromEdges.end(); ++itr) {
			toReturn << "Edge # " << edgeCounter++ << " is at address " << *itr << " and has cost of " << (*itr)->cost << "\n";
		}
		
		toReturn << "\n";
		edgeCounter = 0;
		
		toReturn << "There are " << toEdges.size() << " outgoing edges leading from this CGNode\n";
		
		for (itr = toEdges.begin(); itr != toEdges.end(); ++itr) {
			toReturn << "Edge # " << edgeCounter++ << " is at address " << *itr << " and has cost of " << (*itr)->cost << "\n";
			toReturn << "goes to " << (*itr)->to << "\n";

		}
		
		toReturn << "\n";
		

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "\n";

		return toReturn.str();

	}
	
	/**
	* CGNodeContent definitions
	*/
	
	CGNodeContent::CGNodeContent(int inputContent) {
	
		content = inputContent;

	}

	/**
	* CGEdge definitions
	*/

	CGEdge::CGEdge(CGNode* fromNode, CGNode* toNode, int inputCost) {

		from = fromNode;
		to = toNode;
		cost = inputCost;

	}

	std::string CGEdge::getDescription(bool doPadding) {

		std::stringstream toReturn;

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "CGEdge with address of: " << this << "\n";
		toReturn << "Edge goes from node " << from << " to node " << to << "\n";
		toReturn << "Edge has cost of " << cost << "\n";

		if (doPadding) {
			toReturn << padString;
		}
		
		toReturn << "\n";

		return toReturn.str();

	}

	/**
	* CGGraph definitions
	* */

	CGGraph::CGGraph(eSSA* owner, std::string inputFuncName) {
		this->owner = owner;
		funcName = inputFuncName;
	}
	
	void CGGraph::addConstraint(Instruction* inputInstr) {

		if (D) errs() << "addConstraint - " << *inputInstr << "\n";
		
		//CONTROL FLOW
		if (isa<PHINode>(inputInstr)) {
			constraints.push_back(new CGConstraint(CGConstraint::CONTROL_FLOW, inputInstr));
		} 
		// C1/C2
		else if (StoreInst* storeInst = dyn_cast<StoreInst>(inputInstr)) {
			if (ConstantInt *constInt = dyn_cast<ConstantInt>(storeInst->getValueOperand())) {
				if (storeInst->getPointerOperand()->hasName()) {
					int intValue = constInt->getSExtValue();
					std::vector<int>::iterator it;
					for (it = arrayLengths.begin(); it != arrayLengths.end(); ++it) {
						if ((*it) == intValue) {
							if (D) errs() << "pushing back C1 \n";
							constraints.push_back(new CGConstraint(CGConstraint::C1, inputInstr));
							break;
						}
					}
					if (D) errs() << "pushing back C2 from store with constant int\n";
					constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
				}
			}
			else {
				if (D) errs() << "pushing back C2 from store with variable\n";
				constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));  //storeInst->getValueOperand() may be a variable
			}
		}
		else if (LoadInst* loadInst = dyn_cast<LoadInst>(inputInstr)) {
				if (D) errs() << "pushing back C2 from load\n";
				constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
		}
		else if (CastInst* castInst = dyn_cast<CastInst>(inputInstr)) {
			if (D) errs() << "pushing back C2 from cast\n";
			constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
		}
		//C3		
		else {
			if (D) errs() << "pushing back C3\n";
			constraints.push_back(new CGConstraint(CGConstraint::C3, inputInstr));
		}
	
	}
	
	void CGGraph::addConstraint(Instruction* inputInstr, std::vector<piAssignment*> inputPi1, std::vector<piAssignment*> inputPi2) {
		//This is C4 - there are two vectors of piAssignments, one for each eSSA branch.
		//Each vector will be sizes one or two

		if (D) errs() << "addConstraint - " << *inputInstr << "\n";
		if (D) errs() << "pushing back C4\n";
		constraints.push_back(new CGConstraint(CGConstraint::C4, inputInstr, inputPi1, inputPi2));	
	
	}

	void CGGraph::addConstraint(Instruction* inputInstr, piAssignment* inputPi) {
	
		//This is C5 - there is one and only one PA in any case of C5
		if (D) errs() << "addConstraint - " << *inputInstr << "\n";

		std::vector<piAssignment*> pis;
		pis.push_back(inputPi);
		if (D) errs() << "pushing back C5\n";
		constraints.push_back(new CGConstraint(CGConstraint::C5, inputInstr, pis));	
	}
	
	void CGGraph::solve(std::vector<CallInst*> boundsChecks) {

		errs() << "CGGraph::solve\n";

		std::vector<CallInst*>::iterator callIt;
		std::string varOne;
		std::string varTwo;
		bool rCheck;

		int totalChecked = 0;
		int totalRemoved = 0;

		vector<CallInst*> toRemove;
		for (callIt = boundsChecks.begin(); callIt != boundsChecks.end(); ++callIt) {

			if(std::find(owner->call_insts_removed.begin(), owner->call_insts_removed.end(), (*callIt)) != owner->call_insts_removed.end()) 
			{
				continue;
			} 
			else 
			{
		    	
				if ( (*callIt)->getParent()->getParent()->getName() == this->funcName) {
					
					errs() << "Got in!!!\n";

					Value *v0 = (*callIt)->getArgOperand(0);
					Value *v1 = (*callIt)->getArgOperand(1);
					if (ConstantInt *c0 = dyn_cast<ConstantInt>(v0)) {
						if (ConstantInt *c1 = dyn_cast<ConstantInt>(v1)) {
							totalChecked++;
							if ((c0->getSExtValue() >= 0) && (c0->getSExtValue() < c1->getSExtValue())) {
								//trivial constant case
								totalRemoved++;
								errs() << "Array bounds check is TRIVIALLY redundant, instruction should be removed!\n";
								errs() << **callIt << "\n";
								errs() << "c0 = " << c0->getSExtValue() << "c1 = " << c1->getSExtValue() << "\n";
								toRemove.push_back(*callIt);
								
							}
						} 
					} 
					else {
					
						varOne = getNameFromValue((*callIt)->getArgOperand(1), (*callIt)); //length of array
						//varTwo = owner->check_pi_assignments[*callIt]->getAssignedName();  //indexing variable
						varTwo = getNameFromValue((*callIt)->getArgOperand(0), (*callIt)); //indexing variable

				
						errs() << "Func name is " << funcName << "\n";
						errs() << "First one is " << varOne << "\nSecond one is " << varTwo << "\n";
						if (hasNode(varOne)) { errs() << "Has first node!\n";}
						if (hasNode(varTwo)) { errs() << "Has second node!\n";}

						if (hasNode(varOne) && hasNode(varTwo)) {
							rCheck = doTraverse(varOne, varTwo);
							totalChecked++;
							if (rCheck) {
								totalRemoved++;
								//errs() << "Array bounds check is redundant, instruction should be removed!\n";
								toRemove.push_back(*callIt);
							} else {
								//errs() << "Array bounds check is NOT redundant! Instruction should not be removed...\n";
							}

						}
					}
				}
			}
			//}
			//}
		}

		// Remove them outside the traversal loop so you don't screw the iterator.
		for(size_t i = 0; i < toRemove.size(); i++)
		{

			CallInst *deadCall = toRemove[i];
			deadCall->eraseFromParent();
			owner->call_insts_removed.push_back(deadCall);
		}

		errs() << "Total number of checks performed for function: " << totalChecked << "\n";
		errs() << "Total number of checks removed for function: " << totalRemoved << "\n";

	}

	bool CGGraph::doTraverse(std::string firstName, std::string secondName) {

		CGNode* firstNode = getNode(firstName);
		CGNode* secondNode = getNode(secondName);
		//CGNode* firstNode = getNode("0");
		//CGNode* secondNode = getNode("tmp70");

		CGTraversal* firstTraverse = new CGTraversal(firstNode);

		continueTraversal(firstTraverse, secondNode);

		std::vector<CGTraversal*>::iterator travIt;

		if (traversals.size() == 0) {
			return false;
		}

		for (travIt = traversals.begin(); travIt != traversals.end(); ++travIt) {
			if ((*travIt)->hasPositiveLoop) {
				return false;
			} else if ((*travIt)->cost > -1) {
				errs() << "cost = " << (*travIt)->cost << "\n";
				return false;
			}
		}

		return true;

	}

	void CGGraph::continueTraversal(CGTraversal* inputTraversal, CGNode* targetNode) {

		if (inputTraversal->nodes.back() == targetNode) { //Check if we have arrived at the target node
			inputTraversal->foundTarget = true;
			traversals.push_back(inputTraversal);
		} else if (inputTraversal->nodes.back()->toEdges.size() > 0) { //There are outbound edges to continue traversing along
			handleLoop(inputTraversal); //Sets boolean to true if positive loop is found. Must continue traversal as we must be able to reach targetNode to matter

			//Check to see if traversal is still possible after loop investigation
			std::vector<CGEdge*>::iterator edgeIt;
			std::vector<CGNode*>::iterator nodeIt;
			bool canTraverse;

			for (edgeIt = inputTraversal->nodes.back()->toEdges.begin(); edgeIt != inputTraversal->nodes.back()->toEdges.end(); ++edgeIt) {
				canTraverse = true;
				for (nodeIt = inputTraversal->noTraverse.begin(); nodeIt != inputTraversal->noTraverse.end(); ++nodeIt) {
					if ((*edgeIt)->to == (*nodeIt)) {
						canTraverse = false; //Traversing on this edge would result in a loop
					}
				}
				if (canTraverse) {
					CGTraversal* newTraversal = new CGTraversal(inputTraversal->nodes, inputTraversal->cost, inputTraversal->hasPositiveLoop, inputTraversal->noTraverse);
					newTraversal->nodes.push_back((*edgeIt)->to);
					newTraversal->cost += (*edgeIt)->cost;
					continueTraversal(newTraversal, targetNode);
				}
			}


		}

	}

	void CGGraph::handleLoop(CGTraversal* inputTraversal) {

		std::vector<CGEdge*>::iterator edgeIt;
		std::vector<CGNode*>::iterator nodeIt;

		for (edgeIt = inputTraversal->nodes.back()->toEdges.begin(); edgeIt != inputTraversal->nodes.back()->toEdges.end(); ++edgeIt) {
			for (nodeIt = inputTraversal->nodes.begin(); nodeIt != inputTraversal->nodes.end(); ++nodeIt) {
				if ((*edgeIt)->to == (*nodeIt)) { //ZOMG loop found
					std::vector<CGNode*> subLoop;
					copy(nodeIt, inputTraversal->nodes.end(), back_inserter(subLoop));
					subLoop.push_back((*edgeIt)->to);
					if (getLoopCost(subLoop) > 0) {
						inputTraversal->hasPositiveLoop = true;
					}
					inputTraversal->noTraverse.push_back((*edgeIt)->to);
				}
			}
		}

		
	}

	int CGGraph::getLoopCost(std::vector<CGNode*> inputNodes) {

		std::vector<CGNode*>::iterator nodeIt;
		std::vector<CGEdge*>::iterator edgeIt;
		int loopCost = 0;

		if (inputNodes.size() <= 1) {
			errs() << "Hey now, you shouldn't be passing vectors of length 1 to getLoopCost!\n";
			return 0; //THIS SHOULD NEVER BE HIT!
		}

		for (nodeIt = inputNodes.begin(); nodeIt != inputNodes.end() - 1; ++nodeIt) {
			for (edgeIt = (*nodeIt)->toEdges.begin(); edgeIt != (*nodeIt)->toEdges.end(); ++edgeIt) {
				if ((*edgeIt)->to == (*nodeIt)) {
					loopCost += (*edgeIt)->cost;
				}
			}
		}

		return loopCost;

	}

	CGNode* CGGraph::getNode(std::string nodeName) {

		std::map<std::string, CGNode*>::iterator it;

		for (it = nodes.begin(); it != nodes.end(); ++it) {
			if (it->first.compare(nodeName) == 0) {
				return it->second;
			}
		}

		CGNodeContent* newContent = new CGNodeContent(0); //TODO add value if needed
		CGNode* newNode = new CGNode(newContent);
		nodes.insert(make_pair(nodeName, newNode));
		return newNode;

	}

	void CGGraph::constructGraph() {

		errs() << "CGGraph::constructGraph() \n";

		std::vector<CGConstraint*>::iterator it;
		std::string nodeName;
		CGConstraint* curConstraint;
		CGNode* firstNode;
		CGNode* secondNode;
		std::vector<int>::iterator lengthIt;
		int curValue;
		ConstantInt* cInt;

		for (it = constraints.begin(); it != constraints.end(); ++it) {

			curConstraint = *it;
			if (D) errs() << "\n";
			if (D) errs() << "Constraint at PP: " << *(curConstraint->programPoint) << "\n";
			Instruction* PP = curConstraint->programPoint;

			switch(curConstraint->type) {
				case CGConstraint::C1:
				{
					//Done and ready to test
					if (D) errs() << "CGConstraint::C1 \n";

					firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(0), PP));
					secondNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));

					firstNode->connectTo(secondNode, 0);

					/*
					std::stringstream ss;
					ss << curConstraint->arrayLength;
					firstNode = getNode(ss.str());
					secondNode = getNode(
					firstNode = getNode(curConstraint->arrayLength);

					for (lengthIt = arrayLengths.begin(); lengthIt != arrayLengths.end(); ++lengthIt) {
						if ((*lengthIt) == curConstraint->arrayLength)
						if (lengthIt->second == curConstraint->arrayLength) {
							std::stringstream ss;
							ss << curConstraint->arrayLength;
							firstNode = getNode(ss.str());
							//firstNode = getNode(lengthIt->first); //Array name
							secondNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));
							firstNode->connectTo(secondNode, 0);
						} 
					}
					*/
					break;
				}
				case CGConstraint::C2:
				{
					if (D) errs() << "CGConstraint::C2 \n";
					if (StoreInst* storeInst = dyn_cast<StoreInst>(PP)) {
						/*
						operand(0) = constant int or variable of int type
						operand(1)  = var being stored into
						*/
						firstNode = getNode(getNameFromValue(PP->getOperand(0), PP));
						secondNode = getNode(getNameFromValue(PP->getOperand(1), PP));
						firstNode->connectTo(secondNode, 0);
					}
					else if (LoadInst* loadInst = dyn_cast<LoadInst>(PP)) {
						/*
						operand(0) = pointer being loaded from
						(Value*)PP  = var being loaded into
						*/
						firstNode = getNode(getNameFromValue(PP->getOperand(0), PP));
						secondNode = getNode(getNameFromValue(PP, PP));
						firstNode->connectTo(secondNode, 0);
					}
					else if (CastInst* castInst = dyn_cast<CastInst>(PP)) {
						/*
						operand(0) = var being casted 
						(Value*)PP  = var getting the result of the cast
						*/
						firstNode = getNode(getNameFromValue(PP->getOperand(0), PP));
						secondNode = getNode(getNameFromValue(PP, PP));
						firstNode->connectTo(secondNode, 0);
					}

					break;
				}
				case CGConstraint::C3:
				{
					//Done and ready to test
					if (D) errs() << "CGConstraint::C3 \n";

					secondNode = getNode(getNameFromValue(PP, PP));
					if ((cInt = dyn_cast<ConstantInt>(curConstraint->programPoint->getOperand(0)))) {
						firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));
					} else if ((cInt = dyn_cast<ConstantInt>(curConstraint->programPoint->getOperand(1)))) {
						firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(0), PP));
					} else {
						errs() << "Could not cast to constant int within graph construction method\n";
						return;
					}
					curValue = cInt->getSExtValue();
					firstNode->connectTo(secondNode, curValue);
					break;
				}
				case CGConstraint::C4:
				{
				
					if (D) errs() << "CGConstraint::C4 \n";

					CmpInst* cmpInst;
					BranchInst* branchInst;
					
					if( !(branchInst = dyn_cast<BranchInst>(curConstraint->programPoint)) ) {
						errs() << "ERROR: BranchInst cast unsuccessful for C4 in CGGraph::constructGraph() \n";
						return;
					}
					if( !(cmpInst = dyn_cast<CmpInst>(owner->br_to_cmp[branchInst])) ) {
						errs() << "ERROR: CmpInst not found for C4 in CGGraph::constructGraph() \n";
						return;
					}
					if (D) errs() << "cmpInst: " << *cmpInst << "\n";

					int size1, size2;
					size1 = curConstraint->piAssignments.size();
					size2 = curConstraint->piAssignments2.size();
					/*
					There are two possibilities here: (a) size1 = size2 = 2, or (b) size1 = size2 = 1;
					If (b), there will be one operand in the compare inst that is a literal value.
					That literal value still generates a constraint although it does not generate a pi assignment.
					We need to account for that.
					*/

					if (size1 != size2) {
						errs() << "ERROR: piAssignments not of equal length for C4 in CGGraph::constructGraph()\n"; 
						return;
					}
					if ( (size1 < 1) || (size1 > 2) ) {
						errs() << "ERROR: piAssignments.size() != 1 or 2 for C4 in CGGraph::constructGraph()\n"; 
						return;
					}
					if (D) errs() << "number of piAssignments: " << size1 << "\n";
					if (D) errs() << "number of piAssignments2: " << size2 << "\n";
						
					//this takes care of the first two constraints for both cases (size = 1 or size = 2)
					//first branch
					for (int i = 0; i < size1; ++i) {
						firstNode = getNode(curConstraint->piAssignments[i]->getOperandName()); //vi - wr
						secondNode = getNode(curConstraint->piAssignments[i]->getAssignedName()); //vj - ws 
						firstNode->connectTo(secondNode, 0); //vi -> vj 0  -  wr -> ws 0
					}
					//second branch
					for (int i = 0; i < size2; ++i) {
						firstNode = getNode(curConstraint->piAssignments2[i]->getOperandName()); //vi - wr
						secondNode = getNode(curConstraint->piAssignments2[i]->getAssignedName()); //vk - wt 
						firstNode->connectTo(secondNode, 0); //vi -> vk 0  -  wr -> wt 0
					}
		
					/*
					- first and second op names for the third constraint for branches 1 and 2
					- each case is stored in the order of the pi assignments, which is also the 
					order of the cmp instruction operands
					*/
					std::string firstOpNameBr1, secondOpNameBr1, firstOpNameBr2, secondOpNameBr2;
					
					if (size1 == 1) {
						/*
						the 2nd constraint in the table will not exist in this case, but there will be a 3rd constraint
						that was missed by the above, e.g.
						if (x1 <= 10)
						1. x2 <= x1
						2. nothing
						3. x2 <= 10  <---- we need to add this here
						*/ 
						
						//prune the int from the CmpInst
						if (cmpInst->getNumOperands() != 2) {
							errs() << "ERROR: cmpInst->getNumOperands() != 2 in CGGraph::constructGraph()\n";
							return;
						}
						if ((cInt = dyn_cast<ConstantInt>(cmpInst->getOperand(0)))) {
							//int is first op
							firstOpNameBr1 = getNameFromValue(cInt, PP);
							secondOpNameBr1 = curConstraint->piAssignments[0]->getAssignedName();
							firstOpNameBr2 = firstOpNameBr1;
							secondOpNameBr2 = curConstraint->piAssignments2[0]->getAssignedName();
						} else if ((cInt = dyn_cast<ConstantInt>(cmpInst->getOperand(1)))) {
							//int is second op 
							firstOpNameBr1 = curConstraint->piAssignments[0]->getAssignedName();
							secondOpNameBr1 = getNameFromValue(cInt, PP);
							firstOpNameBr2 = curConstraint->piAssignments2[0]->getAssignedName();
							secondOpNameBr2 = secondOpNameBr1;
						} else {
							errs() << "ERROR: int not found in cmpInstr in CGGraph::constructGraph()\n";
							return;
						}
					}
					else if (size1 == 2) {
						//store in order of pi assignments
						firstOpNameBr1 = curConstraint->piAssignments[0]->getAssignedName();
						secondOpNameBr1 = curConstraint->piAssignments[1]->getAssignedName();
						firstOpNameBr2 = curConstraint->piAssignments2[0]->getAssignedName();
						secondOpNameBr2 = curConstraint->piAssignments2[1]->getAssignedName();
					}
	
					if (D) {
						errs() << "firstOpNameBr1 = " << firstOpNameBr1 << " secondOpNameBr1 = " << secondOpNameBr1 << "\n";
						errs() << "firstOpNameBr2 = " << firstOpNameBr2 << " secondOpNameBr2 = " << secondOpNameBr2 << "\n";
					}

					CGNode* firstNodeBr1 = getNode(firstOpNameBr1);
					CGNode* secondNodeBr1 = getNode(secondOpNameBr1);
					CGNode* firstNodeBr2 = getNode(firstOpNameBr2);
					CGNode* secondNodeBr2 = getNode(secondOpNameBr2);
					
					switch(cmpInst->getPredicate()) {
						case CmpInst::ICMP_SGT: // >
							firstNodeBr1->connectTo(secondNodeBr1, -1); //vj -> ws -1
							secondNodeBr2->connectTo(firstNodeBr2, 0); //wt -> vk 0
							break;
						case CmpInst::ICMP_SLT: // <
							secondNodeBr1->connectTo(firstNodeBr1, -1); //ws -> vj -1
							firstNodeBr2->connectTo(secondNodeBr2, 0); //vk -> wt 0
							break;
						case CmpInst::ICMP_SGE: // >=
							firstNodeBr1->connectTo(secondNodeBr1, 0); //vj -> ws 0
							secondNodeBr2->connectTo(firstNodeBr2, -1); //wt -> vk -1
							break;
						case CmpInst::ICMP_SLE: // <=
							secondNodeBr1->connectTo(firstNodeBr1, 0); //ws -> vj 0
							firstNodeBr2->connectTo(secondNodeBr2, -1); //vk -> wt -1						
							break;
						default:
							errs() << "Construction method fell through in C4. Predicate was " << cmpInst->getPredicate() << "\n";
					}

					break;
				}
				case CGConstraint::C5:
				{
					if (D) errs() << "CGConstraint::C5 \n";
					//operand 1 = array length
					firstNode = getNode(getNameFromValue(PP->getOperand(1), PP));
					secondNode = getNode(curConstraint->piAssignments[0]->getAssignedName());
					firstNode->connectTo(secondNode, -1);
					break;
				}
				case CGConstraint::CONTROL_FLOW:
				{
					//Done and ready to test
					if (D) errs() << "CGConstraint::CONTROL_FLOW \n";

					firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(0), PP));
					secondNode = getNode(getNameFromValue(curConstraint->programPoint, PP));
					firstNode->connectTo(secondNode, 0);
					firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));
					firstNode->connectTo(secondNode, 0);
					break;
				}
			} //end switch
		} //end for	
	} //end constructGraph()

	std::string CGGraph::getNameFromValue(Value* inputValue, Instruction* inst) {

		if (D) errs() << "getNameFromValue\n";
		if (D) errs() << "original name = " << inputValue->getName().str() << "\n";

		std::string toCompare = inputValue->getName().str();

		if (toCompare.empty()) {
			if (ConstantInt* cInt = dyn_cast<ConstantInt>(inputValue)) {
				std::stringstream sStream;
				sStream << cInt->getSExtValue();
				if (D) errs() << "name given to int = " << sStream.str() << "\n";
				return sStream.str();
			} else {
				errs() << "getNameFromValue received something that wasn't a constant int: " << *inputValue;
				return "nullNode";
			}
		} else {

			std::string newName = owner->get_mapped_name(toCompare, inst); 
			if (D) errs() << "mapped name = " << newName << "\n";
			return newName;
		}

	}
	
	void CGGraph::describeInstruction(Instruction* inputInstr) {

		errs() << "----------------------------------------------------------\n";
		errs() << "Instruction is at address " << inputInstr << "\n";
		errs() << "Instruction has name of " << inputInstr->getName() << "\n";
		errs() << "Instruction is of type " << inputInstr->getOpcodeName() << "\n";
		errs() << "Instruction debug dump: ";
		inputInstr->dump();
		errs() << "\n";

		uint index;
		uint numOperands = inputInstr->getNumOperands();

		errs() << "# of operands in instruction: " << numOperands << "\n";		

		Value* curOperand;

		for (index = 0; index < inputInstr->getNumOperands(); index++) {
			errs() << "\n";
			curOperand = inputInstr->getOperand(index);
			errs() << "Information for operand # " << index << "\n";
			errs() << "Operator debug dump: ";
			curOperand->dump();
			errs() << "Operand name: " << curOperand->getName() << "\n";
			errs() << "Operand type: " << curOperand->getType()->getTypeID() << " (";
			curOperand->getType()->dump();
			errs() << ")\n";
		}
		
		errs() << "\n";

	}

	std::string CGGraph::getDescription(bool doPadding) {

		std::stringstream toReturn;

		if (doPadding) {
				toReturn << padString;
		}

		toReturn << "Information for graph " << this << "\n";
		toReturn << "Relevant function name is " << funcName << "\n";
		toReturn << "Total # of edges " << edges.size() << "\n";
		toReturn << "Total # of nodes " << nodes.size() << "\n";

		std::map<std::string, CGNode*>::iterator nodeIt;
		int loopNum = 0;
		
		for (nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt) {
			toReturn << "Information for node #" << loopNum++ << "\n";
			toReturn << "Node name is " << nodeIt->first << "\n";
			toReturn << "Node information is as follows:\n";
			toReturn << nodeIt->second->getDescription(doPadding) << "\n";
		}

		if (doPadding) {
			toReturn << padString;
		}

		return toReturn.str();

	}

	bool CGGraph::hasNode(std::string inputName) {

		std::map<std::string, CGNode*>::iterator mapIt;

		for (mapIt = nodes.begin(); mapIt != nodes.end(); ++mapIt) {

			if (mapIt->first.compare(inputName) == 0) {
				return true;
			}

		}

		return false;

	}

	void CGGraph::addArrayLengths(patterns::AccessMap inputMap) {

		std::map<CallInst*, ArrayAccess*>::iterator mapIt;

		for (mapIt = inputMap.begin(); mapIt != inputMap.end(); ++mapIt) {
			//if (*mapIt->
			arrayLengths.push_back(mapIt->second->getLength());
		}

	}

	CGLoop::CGLoop(std::vector<CGNode*> inputNodes, int inputCost) {

		nodes = inputNodes;
		cost = inputCost;

	}

	CGTraversal::CGTraversal(CGNode* startNode) {
		nodes.push_back(startNode);
		cost = 0;
		foundTarget = false;
		hasPositiveLoop = false;
	}

	CGTraversal::CGTraversal(std::vector<CGNode*> inputNodes, int inputCost, bool inputHasLoop, std::vector<CGNode*> noTraverseNodes) {
		nodes = inputNodes;
		cost = inputCost;
		foundTarget = false;
		hasPositiveLoop = inputHasLoop;
		noTraverse = noTraverseNodes;
	}

	std::string CGTraversal::getDescription() {

		std::stringstream ss;
		std::vector<CGNode*>::iterator nodeIt;
		int nodeCount = 0;

		ss << "Traversal at address " << this << "\n";
		ss << "Traversal has a total of " << nodes.size() << " nodes\n";
		for (nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt) {
			ss << "Node #" << nodeCount++ << " is at address " << (*nodeIt) << "\n";
		}
		ss << "Traversal has cost of " << cost << "\n";

		return ss.str();

	}

