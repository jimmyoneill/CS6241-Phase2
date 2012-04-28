#include "BoundsCheckCodeGenerator.h"
#include "llvm/Constants.h"
#include "llvm/Function.h"
#include "llvm/Support/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

namespace codegen
{
	BoundsCheckCodeGenerator::BoundsCheckCodeGenerator(Module *mod)
	{
		this->checkFuncName = mangleName("check_", 7);
		this->module = mod;
	}

	string BoundsCheckCodeGenerator::getCheckFuncName()
	{
		return this->checkFuncName;
	}

    CallInst *BoundsCheckCodeGenerator::insertArrayBoundsCheck(GetElementPtrInst *inst, ArrayAccess *access)
    {
    	// Set up arguments.
		vector<Value*> args;
		args.push_back(access->getIndex());
		args.push_back(
			ConstantInt::getSigned(
				IntegerType::get(this->module->getContext(), 64), 
				access->getLength()));

		return this->insertArrayBoundsCheck(inst, args);
    }

	CallInst *BoundsCheckCodeGenerator::insertArrayBoundsCheck(
		GetElementPtrInst *gep, 
		vector<Value*> args)
	{
		llvm::LLVMContext &context = this->module->getContext();
		llvm::IRBuilder<> builder(context);
		builder.SetInsertPoint(gep);

		// Grab the function
		Function* checkFunc = this->module->getFunction(this->getCheckFuncName());
		if(checkFunc != NULL)
		{
			return builder.CreateCall(checkFunc, args);
		}
		else
		{
			// Fatal error!
			string msg = "Check function doesn't exist in the module! Cannot continue!\n";
			errs() << msg;
			assert(msg);
		}

		return NULL;
	}

	void BoundsCheckCodeGenerator::insertFuncDefinitions() 
    {
		// Check function type/args
		std::vector<Type*>checkFuncArgs;
		checkFuncArgs.push_back(IntegerType::get(this->module->getContext(), 64));
		checkFuncArgs.push_back(IntegerType::get(this->module->getContext(), 64));
		FunctionType* checkFuncType = FunctionType::get(
			Type::getVoidTy(this->module->getContext()),
			checkFuncArgs,
			false);

		PointerType* PointerTy_3 = PointerType::get(IntegerType::get(this->module->getContext(), 8), 0);

		// Printf function type/args
		std::vector<Type*>printFuncArgs;
		printFuncArgs.push_back(PointerTy_3);
		FunctionType* printFuncType = FunctionType::get(
			IntegerType::get(this->module->getContext(), 32),
			printFuncArgs,
			false);

		// Exit function type/args
		std::vector<Type*>exitFuncArgs;
		exitFuncArgs.push_back(IntegerType::get(this->module->getContext(), 32));
		FunctionType* exitFuncType = FunctionType::get(
			Type::getVoidTy(this->module->getContext()),
			exitFuncArgs,
			false);

		// Function Declarations
		Function* checkFunc = this->module->getFunction(this->getCheckFuncName());
		if (!checkFunc) 
		{
			checkFunc = Function::Create(
				checkFuncType,
				GlobalValue::ExternalLinkage,
				this->getCheckFuncName(), this->module); 
			
			checkFunc->setCallingConv(CallingConv::C);
		}

		AttrListPtr checkFuncPal;
		{
			SmallVector<AttributeWithIndex, 4> Attrs;
			AttributeWithIndex PAWI;
			PAWI.Index = 4294967295U; 
			PAWI.Attrs = 0  | Attribute::NoUnwind | Attribute::UWTable;
			Attrs.push_back(PAWI);
			checkFuncPal = AttrListPtr::get(Attrs.begin(), Attrs.end());
		}

		checkFunc->setAttributes(checkFuncPal);

		Function* exitFunc = this->module->getFunction("exit");
		if (!exitFunc) 
		{
			exitFunc = Function::Create(
				exitFuncType,
				GlobalValue::ExternalLinkage,
				"exit", 
				this->module); // (external, no body)

			exitFunc->setCallingConv(CallingConv::C);
		}

		AttrListPtr exitFuncPal;
		{
			SmallVector<AttributeWithIndex, 4> Attrs;
			AttributeWithIndex PAWI;
			PAWI.Index = 4294967295U; 
			PAWI.Attrs = 0  | Attribute::NoReturn | Attribute::NoUnwind;
			Attrs.push_back(PAWI);
			exitFuncPal = AttrListPtr::get(Attrs.begin(), Attrs.end());
		}

		exitFunc->setAttributes(exitFuncPal);

		Function* printFunc = this->module->getFunction("puts");
		if (!printFunc) 
		{
			printFunc = Function::Create(
				printFuncType,
				GlobalValue::ExternalLinkage,
				"puts", 
				this->module); // (external, no body)
			printFunc->setCallingConv(CallingConv::C);
		}

		AttrListPtr printFuncPal;
		{
			SmallVector<AttributeWithIndex, 4> Attrs;
			AttributeWithIndex PAWI;
			PAWI.Index = 1U; 
			PAWI.Attrs = 0  | Attribute::NoCapture;
			Attrs.push_back(PAWI);
			PAWI.Index = 4294967295U; 
			PAWI.Attrs = 0  | Attribute::NoUnwind;
			Attrs.push_back(PAWI);
			printFuncPal = AttrListPtr::get(Attrs.begin(), Attrs.end());
		}

		printFunc->setAttributes(printFuncPal);

		// Global Variable Declarations
		ArrayType* charArrayType = ArrayType::get(IntegerType::get(this->module->getContext(), 8), 27);
		GlobalVariable* errMsgGlobalPtr = new GlobalVariable(
			*(this->module), 
			charArrayType,
			true,
			GlobalValue::InternalLinkage,
			0,
			mangleName("str_", 5));

		// Constant Definitions
		Constant* errMsg = ConstantArray::get(this->module->getContext(), "Array index out of bounds!", true);

		std::vector<Constant*> intConstants;
		ConstantInt* zeroAsInt64 = ConstantInt::get(this->module->getContext(), APInt(64, StringRef("0"), 10));
		ConstantInt* negOneAsInt32 = ConstantInt::get(this->module->getContext(), APInt(32, StringRef("-1"), 10));
		ConstantInt* negOneAsInt64 = ConstantInt::get(this->module->getContext(), APInt(64, StringRef("-1"), 10));
		intConstants.push_back(zeroAsInt64);
		intConstants.push_back(zeroAsInt64); // Not sure why we push this twice

		Constant* constantsPtr = ConstantExpr::getGetElementPtr(errMsgGlobalPtr, intConstants);

		errMsgGlobalPtr->setInitializer(errMsg);

		// Function: check (checkFunc)
		{
			Function::arg_iterator args = checkFunc->arg_begin();
			Value* idxInt64 = args++;
			idxInt64->setName("idx");
			Value* lenInt64 = args++;
			lenInt64->setName("len");

			BasicBlock* entryBlock = BasicBlock::Create(this->module->getContext(), "",checkFunc,0);
			BasicBlock* printErrMsgAndExitBlock = BasicBlock::Create(this->module->getContext(), "",checkFunc,0);
			BasicBlock* exitBlock = BasicBlock::Create(this->module->getContext(), "",checkFunc,0);

			// Block  (entryBlock)
			ICmpInst* lowerBoundsCheck = new ICmpInst(*entryBlock, ICmpInst::ICMP_SGT, idxInt64, negOneAsInt64, "");
			ICmpInst* upperBoundsCheck = new ICmpInst(*entryBlock, ICmpInst::ICMP_SLT, idxInt64, lenInt64, "");
			BinaryOperator* lowerOrUpper = BinaryOperator::Create(Instruction::And, lowerBoundsCheck, upperBoundsCheck, "or.cond", entryBlock);
			BranchInst::Create(exitBlock, printErrMsgAndExitBlock, lowerOrUpper, entryBlock);

			// Block  (printErrMsgAndExitBlock)
			CallInst* printCall = CallInst::Create(printFunc, constantsPtr, "puts", printErrMsgAndExitBlock);
			printCall->setCallingConv(CallingConv::C);
			printCall->setTailCall(true);
			AttrListPtr printCallPal;
			printCall->setAttributes(printCallPal);

			CallInst* exitCall = CallInst::Create(exitFunc, negOneAsInt32, "", printErrMsgAndExitBlock);
			exitCall->setCallingConv(CallingConv::C);
			exitCall->setTailCall(true);
			AttrListPtr exitCallPal;
			{
				SmallVector<AttributeWithIndex, 4> Attrs;
				AttributeWithIndex PAWI;
				PAWI.Index = 4294967295U; 
				PAWI.Attrs = 0  | Attribute::NoReturn | Attribute::NoUnwind;
				Attrs.push_back(PAWI);
				exitCallPal = AttrListPtr::get(Attrs.begin(), Attrs.end());
			}

			exitCall->setAttributes(exitCallPal);

			new UnreachableInst(this->module->getContext(), printErrMsgAndExitBlock);
			ReturnInst::Create(this->module->getContext(), exitBlock);
		}
    }

    string mangleName(string toMangle, const int length)
    {
		static const char alphanum[] =
		        "0123456789"
		        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		        "abcdefghijklmnopqrstuvwxyz";

        // Init random seed.
		srand(time(NULL));

        string mangled = toMangle;
        for(int i = 0; i < length; i++)
        {
        	mangled += alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return mangled;
    }
}
