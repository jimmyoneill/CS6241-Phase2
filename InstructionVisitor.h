#ifndef INSTRUCTIONVISITOR_H
#define INSTRUCTIONVISITOR_H

#include "llvm/Instruction.h"
#include "llvm/Instructions.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"

using namespace llvm;

namespace patterns
{
	// Basic visitor pattern. Subclass and override what you need.
	class InstructionVisitor
	{
	private:
		bool isRealFunction(Function *func);

	public:
		// Memory operators
		virtual void visitAtomicCmpXchgInst(AtomicCmpXchgInst *inst);
		virtual void visitAtomicRMWInst(AtomicRMWInst *inst);
		virtual void visitAllocaInst(AllocaInst *inst);
		virtual void visitLoadInst(LoadInst *inst);
		virtual void visitStoreInst(StoreInst *inst);
		virtual void visitGetElementPtrInst(GetElementPtrInst *inst);
		virtual void visitFenceInst(FenceInst *inst);
		
		// Binary operators
		virtual void visitBinaryOperator(BinaryOperator *op);
		
		// Terminator operators
		virtual void visitUnwindInst(UnwindInst *inst);
		virtual void visitResumeInst(ResumeInst *inst);
		virtual void visitReturnInst(ReturnInst *inst);
		virtual void visitSwitchInst(SwitchInst *inst);
		virtual void visitUnreachableInst(UnreachableInst *inst);
		virtual void visitBranchInst(BranchInst *inst);
		virtual void visitIndirectBrInst(IndirectBrInst *inst);
		virtual void visitInvokeInst(InvokeInst *inst);
		
		// Cast operators
		virtual void visitCastInst(CastInst *inst);

		// Other operators
		virtual void visitICmpInst(ICmpInst *inst);
		virtual void visitFCmpInst(FCmpInst *inst);
		virtual void visitCallInst(CallInst *inst);
		virtual void visitPHINode(PHINode *node);
		virtual void visitVAArgInst(VAArgInst *inst);

		// The API.
		void visit(Instruction *inst);
		void visit(BasicBlock *blk);
		void visit(Function *func);
		void visit(Module *mod);
	};
}

#endif
