#ifndef PIASSIGNMENT_H
#define PIASSIGNMENT_H

#include <string>

class piAssignment {

	public:
	std::string assignedBaseName;
	std::string operandBaseName;
	int assignedSubscript;
	int operandSubscript;

	piAssignment(std::string operandBaseName) {
		this->operandBaseName = operandBaseName;
		this->assignedBaseName = operandBaseName;
		//assignedName assignedSubscript and will be set directly during the renaming phase
	}

	std::string getOperandName() { return operandBaseName + int_to_string(operandSubscript); }
	std::string getAssignedName() { return assignedBaseName + int_to_string(assignedSubscript); }
		
	std::string int_to_string(int i) {
		std::string s;
		std::stringstream out;
		out << i;
		s = out.str();
		return s;
	}
};


#endif 
