#include "ConstraintBuilderPass.h"

namespace abcd
{

}

char abcd::ConstraintBuilderPass::ID = 0;
static RegisterPass<abcd::ConstraintBuilderPass> X(
	"essa", 
	"CS6241 phase 2 eSSA ABCD constraint graph check reduction pass", 
	false,
	false);