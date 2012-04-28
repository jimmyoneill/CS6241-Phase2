#ifndef NAIVEBOUNDSCHECKINSERTER_H
#define NAIVEBOUNDSCHECKINSERTER_H

#include "llvm/Pass.h"
#include "llvm/Module.h"
#include "llvm/Function.h"
#include "llvm/BasicBlock.h"
#include "llvm/Instruction.h"
#include "BoundsCheckVisitor.h"
#include "BoundsCheckCodeGenerator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace std;

namespace nbci {
  
  struct NaiveBoundsCheckInserter : public ModulePass {
    private:
      BoundsCheckVisitor *visitor;
      
    public:
      static char ID;
      NaiveBoundsCheckInserter();
      virtual bool runOnModule(Module &M);
      void getAnalysisUsage(AnalysisUsage &AU) const;
      virtual void print(raw_ostream &O, const Module *M) const;
      void printArrayAccessMap(raw_ostream &O) const;
      void printChecks(raw_ostream &O) const;
      BoundsCheckVisitor *getBoundsCheckVisitor() const;
      string getCheckFuncName() const;
  };  
}

#endif
