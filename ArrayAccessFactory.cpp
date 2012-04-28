#include "ArrayAccessFactory.h"

namespace patterns
{
	ArrayAccess *ArrayAccessFactory::create(GetElementPtrInst *gepInst)
	{
		if(PointerType *opType = dyn_cast<PointerType>(gepInst->getPointerOperandType()))
		{	
			if(ArrayType *arrType = dyn_cast<ArrayType>(opType->getElementType()))
			{
				int length = arrType->getNumElements();
				Value *idx = findIndex(gepInst);

				// HACK
				if(idx->getType()->isIntegerTy(64))
				{
					return new ArrayAccess(gepInst, findAllocaInst(gepInst), idx, length);
				}
			}
		}

		return NULL;
	}

	AllocaInst *findAllocaInst(GetElementPtrInst *inst)
	{
		GetElementPtrInst *current = inst;
		while(current != NULL)
		{
			if(AllocaInst *allocInst = dyn_cast<AllocaInst>(current->getPointerOperand()))
			{
				// Return the allocating instruction.
				return allocInst;
			}
			else
			{
				// We're stuck in a chain of GEPs
				current = dyn_cast<GetElementPtrInst>(current->getPointerOperand());
			}
		}

		return NULL;
	}

	Value *findIndex(GetElementPtrInst *inst)
	{
		// Grabs the last item in the list.
		GetElementPtrInst::op_iterator it = inst->idx_end();
		if(it != inst->idx_begin())
		{
			--it;
			return *it;
		}

		return NULL;
	}

	void debugGep(GetElementPtrInst *inst)
	{
		errs() << "DEBUG\n";
		errs() << "Instruction: \t\t";
		inst->dump();
		errs() << "Instruction type: \t";
		inst->getType()->dump(); errs() << "\n";
		errs() << "Instruction is array type?  " << inst->getType()->isArrayTy() << "\n";
		errs() << "Instruction is struct type?  " << inst->getType()->isStructTy() << "\n";
		errs() << "Pointer operand: \t";
		inst->getPointerOperand()->dump();
		errs() << "Pointer operand type: \t  ";
		inst->getPointerOperandType()->dump(); errs() << "\n";
		errs() << "Pointer operand is pointer type? " << inst->getPointerOperandType()->isPointerTy() << "\n";

		if(isa<PointerType>(inst->getPointerOperandType()))
		{
			errs() << "Ptr operand element type: ";
			inst->getPointerOperandType()->getElementType()->dump();
			errs() << "\n";
			errs() << "Pointer operand is struct type?  " << inst->getPointerOperandType()->getElementType()->isStructTy() << "\n";
			errs() << "Pointer operand is array type?   " << inst->getPointerOperandType()->getElementType()->isArrayTy() << "\n";

			if(isa<ArrayType>(inst->getPointerOperandType()->getElementType()))
			{
				ArrayType *arrTy = cast<ArrayType>(inst->getPointerOperandType()->getElementType());
				errs() << "Num Elements " << arrTy->getNumElements() << "\n";
				Value *idx = findIndex(inst);
				errs() << "Index ";
				idx->dump();
			}
		}

		errs() << "Value name " << inst->getName(); errs() << "\n";

		errs() << "END DEBUG\n"; 
	}
}
