#include "InstructionVisitor.h"

namespace patterns
{
	// Is the function not a declaration and not empty?
	bool InstructionVisitor::isRealFunction(Function *func)
	{
		return func != NULL && !(func->empty() || func->isDeclaration());
	}

	// Visits a module.
	void InstructionVisitor::visit(Module *mod)
	{
		if(mod != NULL)
		{
	        for(Module::iterator it = mod->begin(); it != mod->end(); ++it)
	        {                 
	            this->visit(cast<Function>(it));
	        }
    	}
	}

	// Visits a function.
	void InstructionVisitor::visit(Function *func)
	{
		if(this->isRealFunction(func))
		{
			for(Function::iterator it = func->begin(); it != func->end(); ++it)
	    	{
	    		this->visit(cast<BasicBlock>(it));
	    	}
    	}
	}

	// Visits a basic block.
	void InstructionVisitor::visit(BasicBlock *blk)
	{
		if(blk != NULL)
		{
			for(BasicBlock::iterator it = blk->begin(); it != blk->end(); ++it)
			{
				this->visit(cast<Instruction>(it));
			}
		}
	}

	/* Visits an instruction. Will no-op stuff subclasses don't care about. This is also
	 * not an exhaustive implementation. Several operations are skipped because I don't
	 * really care about them at the moment. */
	void InstructionVisitor::visit(Instruction *inst)
	{
		if(inst != NULL)
		{
			switch(inst->getOpcode())
			{
				// Terminators
				case Instruction::Ret:
					this->visitReturnInst(cast<ReturnInst>(inst));
					break;

			  	case Instruction::Br:
			  		this->visitBranchInst(cast<BranchInst>(inst));
			  		break;

			  	case Instruction::Switch: 
			  		this->visitSwitchInst(cast<SwitchInst>(inst));
			  		break;

			  	case Instruction::IndirectBr: 
			  		this->visitIndirectBrInst(cast<IndirectBrInst>(inst));
			  		break;

			  	case Instruction::Invoke: 
			  		this->visitInvokeInst(cast<InvokeInst>(inst));
			  		break;

			  	case Instruction::Resume:
			  		this->visitResumeInst(cast<ResumeInst>(inst));
			  		break;

			  	case Instruction::Unwind:
			  		this->visitUnwindInst(cast<UnwindInst>(inst));
			  		break;

			  	case Instruction::Unreachable:
			  		this->visitUnreachableInst(cast<UnreachableInst>(inst));
			  		break;

				// Standard binary operators
				case Instruction::Add: 
				case Instruction::FAdd:
				case Instruction::Sub: 
				case Instruction::FSub:
				case Instruction::Mul: 
				case Instruction::FMul: 
				case Instruction::UDiv: 
				case Instruction::SDiv:
				case Instruction::FDiv:
				case Instruction::URem:
				case Instruction::SRem:
				case Instruction::FRem:
				case Instruction::And:
				case Instruction::Or: 
				case Instruction::Xor:
				case Instruction::Shl:
				case Instruction::LShr:
				case Instruction::AShr:
			  		this->visitBinaryOperator(cast<BinaryOperator>(inst));
			  		break;

		  		// Memory operators
		  		case Instruction::Alloca:
		  			this->visitAllocaInst(cast<AllocaInst>(inst));
		  			break;

	  			case Instruction::Load:
	  				this->visitLoadInst(cast<LoadInst>(inst));
	  				break;

  				case Instruction::Store:
  					this->visitStoreInst(cast<StoreInst>(inst));
  					break;

				case Instruction::GetElementPtr:
					this->visitGetElementPtrInst(cast<GetElementPtrInst>(inst));
					break;

				case Instruction::Fence:
					this->visitFenceInst(cast<FenceInst>(inst));
					break;

				case Instruction::AtomicCmpXchg:
					this->visitAtomicCmpXchgInst(cast<AtomicCmpXchgInst>(inst));
					break;

				case Instruction::AtomicRMW:
					this->visitAtomicRMWInst(cast<AtomicRMWInst>(inst));
					break;

				// Cast operators
				case Instruction::Trunc:
				case Instruction::ZExt:  
				case Instruction::SExt:
				case Instruction::FPToUI:
				case Instruction::FPToSI:
				case Instruction::UIToFP:
				case Instruction::SIToFP:
				case Instruction::FPTrunc:
				case Instruction::FPExt:  
				case Instruction::PtrToInt:
				case Instruction::IntToPtr:
				case Instruction::BitCast:
					this->visitCastInst(cast<CastInst>(inst));
					break;

				// Everything else
				case Instruction::Call:
					this->visitCallInst(cast<CallInst>(inst));
					break;

				case Instruction::PHI:
					this->visitPHINode(cast<PHINode>(inst));
					break;

				case Instruction::VAArg:
					this->visitVAArgInst(cast<VAArgInst>(inst));
					break;

				case Instruction::ICmp:
					this->visitICmpInst(cast<ICmpInst>(inst));
					break;

				case Instruction::FCmp:
					this->visitFCmpInst(cast<FCmpInst>(inst));
			}
		}
	}

	// Memory operators
	void InstructionVisitor::visitAtomicCmpXchgInst(AtomicCmpXchgInst *inst) { }
	void InstructionVisitor::visitAtomicRMWInst(AtomicRMWInst *inst) { }
	void InstructionVisitor::visitAllocaInst(AllocaInst *inst) { }
	void InstructionVisitor::visitLoadInst(LoadInst *inst) { }
	void InstructionVisitor::visitStoreInst(StoreInst *inst) { }
	void InstructionVisitor::visitGetElementPtrInst(GetElementPtrInst *inst) { }
	void InstructionVisitor::visitFenceInst(FenceInst *inst) { }
	
	// Binary operators
	void InstructionVisitor::visitBinaryOperator(BinaryOperator *op) { }
	
	// Terminator operators
	void InstructionVisitor::visitUnwindInst(UnwindInst *inst) { }
	void InstructionVisitor::visitResumeInst(ResumeInst *inst) { }
	void InstructionVisitor::visitReturnInst(ReturnInst *inst) { }
	void InstructionVisitor::visitSwitchInst(SwitchInst *inst) { }
	void InstructionVisitor::visitUnreachableInst(UnreachableInst *inst) { }
	void InstructionVisitor::visitBranchInst(BranchInst *inst) { }
	void InstructionVisitor::visitIndirectBrInst(IndirectBrInst *inst) { }
	void InstructionVisitor::visitInvokeInst(InvokeInst *inst) { }
	
	// Cast operators
	void InstructionVisitor::visitCastInst(CastInst *inst) { }

	// Other operators
	void InstructionVisitor::visitICmpInst(ICmpInst *inst) { }
	void InstructionVisitor::visitFCmpInst(FCmpInst *inst) { }
	void InstructionVisitor::visitCallInst(CallInst *inst) { }
	void InstructionVisitor::visitPHINode(PHINode *node) { }
	void InstructionVisitor::visitVAArgInst(VAArgInst *inst) { }
}
