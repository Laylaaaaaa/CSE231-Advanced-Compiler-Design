#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"

using namespace std;
using namespace llvm;

namespace {
    struct BranchBias : public FunctionPass {
        static char ID;
        BranchBias() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            Module *mod = F.getParent();
            LLVMContext &context = mod->getContext();

            Function *update_bc = cast<Function>(
                    mod->getOrInsertFunction(
                    "updateBranchInfo", 
                    Type::getVoidTy(context), 
                    Type::getInt1Ty(context)
                    ));
            Function *print_bc = cast<Function>(
                    mod->getOrInsertFunction(
                    "printOutBranchInfo", 
                    Type::getVoidTy(context)
                    ));
            
            for (Function::iterator bb2 = F.begin(); bb2 != F.end(); ++bb2) {
                IRBuilder<> Builder(&*bb2);
                Builder.SetInsertPoint(bb2->getTerminator());
                
                BranchInst *branch_inst = cast<BranchInst>(bb2->getTerminator());

                vector <Value *> argument;

                if (branch_inst != NULL && branch_inst->isConditional()) {
                    argument.push_back(branch_inst->getCondition());
                    Builder.CreateCall(update_bc, argument);
                }

                for (BasicBlock::iterator i1 = bb2->begin(); i1 != bb2->end(); ++i1) {
                    if ((string) i1->getOpcodeName() == "ret") {
                        Builder.SetInsertPoint(&*i1);
                        Builder.CreateCall(print_bc);
                    }
                }
            }

            return false;
        }
    }; 
}

char BranchBias::ID = 0;
static RegisterPass<BranchBias> X("cse231-bb", "Compute Branch Bias",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
