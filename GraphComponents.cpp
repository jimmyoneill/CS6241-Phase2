
#include "GraphComponents.h"
#include "ESSA.h"

using namespace GraphConstruct;
using namespace llvm;

	/**
	* Class CGConstraint
	*
	* This class is used for containing information about the constraints obtained from the control flow graph
	* for a given function.
	*/

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

	/**
	* Class CGNode
	*
	* This class is used to represent the nodes within the constraint graph used to solve the ABCD implementation.
	* These nodes are responsible for keeping track of relations between themselves and their neighbors via vectors
	* of CGEdges
	*/

	CGNode::CGNode() {

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

	/**
	* Class CGEdge
	*
	* This class is used to represent the edges connecting nodes in the constraint graph used to solve the ABCD
	* implementation. Aside from the nodes which the edge connects, this class also holds the traversal cost value
	* for weighted edges.
	*/

	CGEdge::CGEdge(CGNode* fromNode, CGNode* toNode, int inputCost) {

		from = fromNode;
		to = toNode;
		cost = inputCost;

	}

	/**
	* Class CGGraph
	*
	* This class is used to construct the ABCD constraint graph from a vector of CGConstraints. In addition to graph
	* construction, this class also houses the methods for graph traversal and constraint system solving.
	* */

	CGGraph::CGGraph(ESSA* owner, std::string inputFuncName) {
		this->owner = owner;
		totalChecked = 0;
		totalRemoved = 0;
		funcName = inputFuncName;
	}
	
	/**
	* Adds a constraint to the vector of constraints currently held by the graph based on the passed in Instruction.
	* This method is only for constraints C1, C2, C3, and CONTROL_FLOW as constraints C4 and C5 require additional
	* information for proper implementation
	*
	* @param inputInstr - The instruction to construct the appropriate constraint from
	* */
	
	void CGGraph::addConstraint(Instruction* inputInstr) {
		
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
							constraints.push_back(new CGConstraint(CGConstraint::C1, inputInstr));
							break;
						}
					}
					constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
				}
			}
			else {
				constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));  //storeInst->getValueOperand() may be a variable
			}
		}
		else if (isa<LoadInst>(inputInstr)) {
			constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
		}
		else if (isa<CastInst>(inputInstr)) {
			constraints.push_back(new CGConstraint(CGConstraint::C2, inputInstr));
		}
		//C3		
		else {
			constraints.push_back(new CGConstraint(CGConstraint::C3, inputInstr));
		}
	
	}
	
	/**
	* Adds a constraint to the vector of constraints currently held by the graph based on the passed in Instruction
	* as well as the passed in vectors of piAssignment*s. This method is for constraint C4 only.
	* 
	* @param inputInstr - The instruction to construct the appropriate constraint from
	* @param inputPi1 - The first vector of piAssignment* to construct the appropriate constraint from
	* @param inputPi2 - The second vector of piAssignment* to construct the appropriate constraint from
	* */
	
	void CGGraph::addConstraint(Instruction* inputInstr, std::vector<piAssignment*> inputPi1, std::vector<piAssignment*> inputPi2) {
		//This is C4 - there are two vectors of piAssignments, one for each ESSA branch.
		//Each vector will be sizes one or two
		
		constraints.push_back(new CGConstraint(CGConstraint::C4, inputInstr, inputPi1, inputPi2));	
	
	}
	
	/**
	* Adds a constraint to the vector of constraints currently held by the graph based on the passed in Instruction
	* as well as the passed in piAssignment*. This method is for constraint C5 only.
	* 
	* @param inputInstr - The instruction to construct the appropriate constraint from
	* @param inputPi - The pi assignment relating to the passed in instruction
	* */

	void CGGraph::addConstraint(Instruction* inputInstr, piAssignment* inputPi) {
	
		//This is C5 - there is one and only one PA in any case of C5

		std::vector<piAssignment*> pis;
		pis.push_back(inputPi);
		constraints.push_back(new CGConstraint(CGConstraint::C5, inputInstr, pis));
	}
	
	/**
	* Adds all of the array lengths for a given AccessMap to an internally held vector of integers. This array
	* is subsequently used to check upon constraint construction whether or not a given constraint is a C1 as well
	* as a C2
	*
	* @param inputMap - The AccessMap containing all of the array lengths for the relevant function
	* */
	
	void CGGraph::addArrayLengths(patterns::AccessMap inputMap) {

		std::map<CallInst*, ArrayAccess*>::iterator mapIt;

		for (mapIt = inputMap.begin(); mapIt != inputMap.end(); ++mapIt) {
				arrayLengths.push_back(mapIt->second->getLength());
		}

	}
	
	/**
	* Traverses the constructed constraint graph for each array bounds check that is passed in to detect whether
	* or not the given bounds check instruction is redundant.
	*
	* @param boundsChecks - A vector of call instructions that contains all calls to the bounds within the current function
	* */
	
	int CGGraph::solve(std::vector<CallInst*> boundsChecks) {

		std::vector<CallInst*>::iterator callIt;
		std::string varOne;
		std::string varTwo;
		bool rCheck;

		vector<CallInst*> toRemove;
		for (callIt = boundsChecks.begin(); callIt != boundsChecks.end(); ++callIt) {

			if(std::find(owner->callInstsRemoved.begin(), owner->callInstsRemoved.end(), (*callIt)) != owner->callInstsRemoved.end()) 
			{
				continue;
			} 
			else 
			{
		    	
				if ((*callIt)->getParent()->getParent()->getName() == this->funcName) {
					
					Value *v0 = (*callIt)->getArgOperand(0);
					Value *v1 = (*callIt)->getArgOperand(1);
					if (ConstantInt *c0 = dyn_cast<ConstantInt>(v0)) {
						if (ConstantInt *c1 = dyn_cast<ConstantInt>(v1)) {
							totalChecked++;
							if ((c0->getSExtValue() >= 0) && (c0->getSExtValue() < c1->getSExtValue())) {
								//trivial constant case
								totalRemoved++;
								toRemove.push_back(*callIt);
								
							}
						} 
					} 
					else {
					
						varOne = getNameFromValue((*callIt)->getArgOperand(1), (*callIt)); //length of array
						varTwo = getNameFromValue((*callIt)->getArgOperand(0), (*callIt)); //indexing variable

						if (hasNode(varOne) && hasNode(varTwo)) {
							rCheck = doTraverse(varOne, varTwo);
							totalChecked++;
							if (rCheck) {
								totalRemoved++;
								toRemove.push_back(*callIt);
							}

						}
					}
				}
			}
		}

		// Remove them outside the traversal loop so you don't screw the iterator.
		for(size_t i = 0; i < toRemove.size(); i++)
		{
			CallInst *deadCall = toRemove[i];
			deadCall->eraseFromParent();
			owner->callInstsRemoved.push_back(deadCall);
		}

		// errs() << "Total number of checks performed for function: " << totalChecked << "\n";
		// errs() << "Total number of checks removed for function: " << totalRemoved << "\n";

		return totalRemoved;

	}
	
	/**
	* Traverses the graph between two nodes as represented by the input parameters to detect if the
	* traversal cost is beneath the threshold of 0
	*
	* @param firstName - String representation of the node from which to start the traversal
	* @param secondName - String representation of the node which the traversal is attempting to reach
	* @return True if traversal succeeds and cost is < 0, false if otherwise
	* */

	bool CGGraph::doTraverse(std::string firstName, std::string secondName) {

		CGNode* firstNode = getNode(firstName);
		CGNode* secondNode = getNode(secondName);

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
				return false;
			}
		}

		return true;

	}
	
	/**
	* Recursive function that continue a traversal path using vanilla depth-first search in an attempt
	* to reach the passed in targetNode. Upon successfully reaching the target node, the traversal is
	* placed in CGGraph::traversals to be checked against afterwards for instruction removal.
	* 
	* @param inputTraversal - The CGTraversal object that the DFS should continue with
	* @param targetNode - The CGNode that the traversal is trying to reach
	* */

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
	
	/**
	* Checks for loops within the input CGTraversal. However, loops are only checked for against
	* all of the potential next hops in the depth first search. If a loop is found, the edge
	* in the traversal that upon traversing would cause a loop is added to a blacklist vector
	* so that traversal does not happen. Additionally, the loop's cost is checked for as well. If
	* The loop's cost on a single traversal is found to be positive, then a flag is flown within the
	* CGTraversal object that states that it contains a positive loop (ie: if this traversal makes it
	* to the target node, then the bounds check is not redundant)
	*
	* @param inputTraversal - The CGTraversal to check for loops against
	* */

	void CGGraph::handleLoop(CGTraversal* inputTraversal) {

		std::vector<CGEdge*>::iterator edgeIt;
		std::vector<CGNode*>::iterator nodeIt;

		for (edgeIt = inputTraversal->nodes.back()->toEdges.begin(); edgeIt != inputTraversal->nodes.back()->toEdges.end(); ++edgeIt) {
			for (nodeIt = inputTraversal->nodes.begin(); nodeIt != inputTraversal->nodes.end(); ++nodeIt) {
				if ((*edgeIt)->to == (*nodeIt)) { 
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
	
	/**
	* Gets the cost of a given loop
	*
	* @param inputNodes - A vector of the nodes that a loop contains. The first and last elements should be the same
	* @return The cost of a single traversal across the loop
	* */

	int CGGraph::getLoopCost(std::vector<CGNode*> inputNodes) {

		std::vector<CGNode*>::iterator nodeIt;
		std::vector<CGEdge*>::iterator edgeIt;
		int loopCost = 0;

		if (inputNodes.size() <= 1) {
			return 0; //THIS SHOULD NEVER BE HIT!
		}

		for (nodeIt = inputNodes.begin(); nodeIt != inputNodes.end() - 1; ++nodeIt) {
			for (edgeIt = (*nodeIt)->toEdges.begin(); edgeIt != (*nodeIt)->toEdges.end(); ++edgeIt) {
				if ((*edgeIt)->to == (*nodeIt)) {
					loopCost += (*edgeIt)->cost;
					break;
				}
			}
		}

		return loopCost;

	}
	
	/**
	* Returns a CGNode pointer corresponding to the passed in string. If the CGNode does not already exist
	* in the vector of nodes contained by the CGGraph, then the node is created and added to the vector
	* before being returned.
	*
	* @param nodeName - The name of the node you want to retrieve
	* @return CGNode* corresponding to the passed in string
	* */

	CGNode* CGGraph::getNode(std::string nodeName) {

		std::map<std::string, CGNode*>::iterator it;

		for (it = nodes.begin(); it != nodes.end(); ++it) {
			if (it->first.compare(nodeName) == 0) {
				return it->second;
			}
		}

		CGNode* newNode = new CGNode();
		nodeNames.push_back(nodeName);
		nodes.insert(make_pair(nodeName, newNode));
		return newNode;

	}
	
	/**
	* Constructs the constraint graph from the vector of CGConstraints contained by
	* the CGGraph. This is where the majority of the heavy lifting for the CGGraph
	* class is.
	* */

	void CGGraph::constructGraph() {

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
			Instruction* PP = curConstraint->programPoint;

			switch(curConstraint->type) {
				case CGConstraint::C1:
				{

					firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(0), PP));
					secondNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));

					firstNode->connectTo(secondNode, 0);
					
					break;
				}
				case CGConstraint::C2:
				{
					if (isa<StoreInst>(PP)) {
						/*
						operand(0) = constant int or variable of int type
						operand(1)  = var being stored into
						*/
						firstNode = getNode(getNameFromValue(PP->getOperand(0), PP));
						secondNode = getNode(getNameFromValue(PP->getOperand(1), PP));
						firstNode->connectTo(secondNode, 0);
					}
					else if (isa<LoadInst>(PP)) {
						/*
						operand(0) = pointer being loaded from
						(Value*)PP  = var being loaded into
						*/
						firstNode = getNode(getNameFromValue(PP->getOperand(0), PP));
						secondNode = getNode(getNameFromValue(PP, PP));
						firstNode->connectTo(secondNode, 0);
					}
					else if (isa<CastInst>(PP)) {
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
				
					secondNode = getNode(getNameFromValue(PP, PP));
					if ((cInt = dyn_cast<ConstantInt>(curConstraint->programPoint->getOperand(0)))) {
						firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(1), PP));
					} else if ((cInt = dyn_cast<ConstantInt>(curConstraint->programPoint->getOperand(1)))) {
						firstNode = getNode(getNameFromValue(curConstraint->programPoint->getOperand(0), PP));
					} else {
						return;
					}
					curValue = cInt->getSExtValue();
					firstNode->connectTo(secondNode, curValue);
					break;
				}
				case CGConstraint::C4:
				{

					CmpInst* cmpInst;
					BranchInst* branchInst;
					
					if( !(branchInst = dyn_cast<BranchInst>(curConstraint->programPoint)) ) {
						errs() << "ERROR: BranchInst cast unsuccessful for C4 in CGGraph::constructGraph() \n";
						return;
					}
					if( !(cmpInst = dyn_cast<CmpInst>(owner->branchToCompare[branchInst])) ) {
						errs() << "ERROR: CmpInst not found for C4 in CGGraph::constructGraph() \n";
						return;
					}

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
							break;
					}

					break;
				}
				case CGConstraint::C5:
				{
					//operand 1 = array length
					firstNode = getNode(getNameFromValue(PP->getOperand(1), PP));
					secondNode = getNode(curConstraint->piAssignments[0]->getAssignedName());
					firstNode->connectTo(secondNode, -1);
					break;
				}
				case CGConstraint::CONTROL_FLOW:
				{
					//Done and ready to test

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
	
	/**
	* Retrieves a string corresponding to the passed in value and instruction. The instruction
	* is used to access the map of Instructions -> value names as created in the renaming phase
	*
	* @param inputValue - The value for which you want to retrieve the name for
	* @param inst - The instruction that contains the value
	* @return The string corresponding to the mapped value's name
	* */

	std::string CGGraph::getNameFromValue(Value* inputValue, Instruction* inst) {

		std::string toCompare = inputValue->getName().str();

		if (toCompare.empty()) {
			if (ConstantInt* cInt = dyn_cast<ConstantInt>(inputValue)) {
				std::stringstream sStream;
				sStream << cInt->getSExtValue();
				return sStream.str();
			} else {
				return "nullNode";
			}
		} else {

			std::string newName = owner->getMappedName(toCompare, inst); 
			return newName;
		}

	}
	
	/**
	* Checks to see whether or not the CGGraph has a node corresponding to the passed
	* in name.
	*
	* @param inputName - The string representation corresponding to the node to check for
	* @return True if CGGraph has a node corresponding to the string, false otherwise
	* */

	bool CGGraph::hasNode(std::string inputName) {
		
		std::vector<std::string>::iterator sIt;
		
		for (sIt = nodeNames.begin(); sIt != nodeNames.end(); ++sIt) {
			if (sIt->compare(inputName) == 0) {
				return true;
			}
		}

		return false;

	}

	/**
	* Class CGTraversal
	*
	* This class is used as a helper class to keep track of traversals while
	* solving the constraint system.
	*/

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

