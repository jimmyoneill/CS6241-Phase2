#ifndef GRAPHCOMPONENTS_H
#define GRAPHCOMPONENTS_H

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "BoundsCheckVisitor.h"
//#include "eSSA.h"
class piAssignment;
class eSSA;
using namespace llvm;
namespace GraphConstruct {

	/**
	* Forward declarations
	*/

	class CGConstraint;
	class CGNode;
	class CGEdge;
	class CGNodeContent;
	class CGGraph;
	class CGLoop;
	class CGTraversal;

	const std::string padString = "---------------------------------------------------\n";
	const bool D = true;

	class CGConstraint {

		public:
			enum ConType
			{
				C1,
				C2,
				C3,
				C4,
				C5,
				CONTROL_FLOW
			};
			int arrayLength;
			Instruction* programPoint;
			std::vector<piAssignment*> piAssignments;
			std::vector<piAssignment*> piAssignments2; //hackish
			BasicBlock* containerBlock;
			ConType type;
			std::string getDescription(bool doPadding);
			CGConstraint(ConType inputType, Instruction* inputInstr, int inputLength);
			CGConstraint(ConType inputType, Instruction* inputInstr);
			CGConstraint(ConType inputType, Instruction* inputInstr, std::vector<piAssignment*> inputPi);
			CGConstraint(ConType inputType, Instruction* inputInstr, std::vector<piAssignment*> inputPi1, std::vector<piAssignment*> inputPi2); 
	};
	
	class CGNode {
		public:
			std::vector<CGEdge*> fromEdges;
			std::vector<CGEdge*> toEdges;
			CGNodeContent* content;
			std::string getDescription(bool doPadding);
			CGNode(CGNodeContent* inputConst);
			void connectTo(CGNode* toConnect, int edgeValue);
			void connectFrom(CGNode* fromConnect, int edgeValue);
	};
	
	class CGNodeContent {
		public:
			int content;
			CGNodeContent(int inputContent);
	};
	
	class CGEdge {
		public:
			CGNode* from;
			CGNode* to;
			int cost;
			std::string getDescription(bool doPadding);
			CGEdge(CGNode* fromNode, CGNode* toNode, int inputCost);
	};

	class CGGraph {
		public:
			eSSA* owner;
			CGGraph(eSSA* owner, std::string inputFuncName);
			std::vector<CGEdge*> edges;
			std::string funcName;
			std::vector<CGConstraint*> constraints;
			std::vector<int> arrayLengths;
			std::vector<CGTraversal*> traversals;
			std::map<std::string, CGNode*> nodes;
			std::string getDescription(bool doPadding);
			std::string getNameFromValue(Value* inputValue, Instruction* inst);
			CGNode* getNode(std::string nodeName);
			void addConstraint(Instruction* inputInstr);
			void addConstraint(Instruction* inputInstr, piAssignment* inputPi);
			void addConstraint(Instruction* inputInstr, std::vector<piAssignment*> inputPi1, std::vector<piAssignment*> inputPi2);
			void constructGraph(std::vector<CGConstraint*> inputConstraints);
			void constructGraph();
			void solve(std::vector<CallInst*> arrayChecks);
			static void describeInstruction(Instruction* inputInstr);
			bool hasNode(std::string inputName);
			bool doTraverse(std::string firstName, std::string secondName);
			void continueTraversal(CGTraversal* inputTraversal, CGNode* targetNode);
			void addArrayLengths(patterns::AccessMap inputMap);
			void handleLoop(CGTraversal* inputTraversal);
			int getLoopCost(std::vector<CGNode*> inputNodes);
	};

	class CGLoop {
		public:
			std::vector<CGNode*> nodes;
			int cost;
			CGLoop(std::vector<CGNode*> inputNode, int inputCost);
	};

	class CGTraversal {
		public:
			std::vector<CGNode*> nodes;
			int cost;
			CGTraversal(CGNode* startNode);
			CGTraversal(std::vector<CGNode*> inputNodes, int inputCost, bool inputHasLoop, std::vector<CGNode*> noTraverseNodes);
			bool foundTarget;
			bool hasPositiveLoop;
			std::vector<CGNode*> noTraverse;
			std::string getDescription();
	};


	

}

#endif
