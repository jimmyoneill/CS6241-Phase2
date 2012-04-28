#include "ConstraintBuilderPass.h"

namespace abcd
{

}

char abcd::ConstraintBuilderPass::ID = 0;
static RegisterPass<abcd::ConstraintBuilderPass> X(
	"abcd", 
	"CS6241 phase 2 ABCD constraint graph check reduction pass", 
	false,
	false);
