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

#include <map>
#include <vector>

using namespace std;
using namespace llvm;

namespace {
    struct CountDynamicInstructions : public FunctionPass {
        static char ID;
        CountDynamicInstructions() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            Module *mod = F.getParent();
            LLVMContext &context = mod->getContext();

            Function *update_inst = cast<Function>(
                    mod->getOrInsertFunction(
                    "updateInstrInfo", 
                    Type::getVoidTy(context), 
                    Type::getInt32Ty(context), 
                    Type::getInt32PtrTy(context), 
                    Type::getInt32PtrTy(context)
                    ));

            Function *print_inst = cast<Function>(
                    mod->getOrInsertFunction(
                    "printOutInstrInfo", 
                    Type::getVoidTy(context)
                    ));
            
            for (Function::iterator bb1 = F.begin(); bb1 != F.end(); ++bb1) {
                map <int, int> inst_dict;
    
                for (BasicBlock::iterator i1 = bb1->begin(); i1 != bb1->end(); ++i1) {
                    int inst_name = i1->getOpcode();
                    inst_dict[inst_name]++;
                }

                int inst_size = inst_dict.size();
                vector<Constant *> keys;
                vector<Constant *> values;
                vector<Value *> arguments;

                for (map <int, int> ::iterator i2 = inst_dict.begin(); i2 != inst_dict.end(); ++i2) {
                    Constant *key0 = ConstantInt::get(Type::getInt32Ty(context), i2->first);
                    Constant *value0 = ConstantInt::get(Type::getInt32Ty(context), i2->second);
                    keys.push_back(key0);
                    values.push_back(value0);
                }

                ArrayType *arrayTy = ArrayType::get(
                        Type::getInt32Ty(context), 
                        inst_size);
                GlobalVariable *key1 = new GlobalVariable(
                    *mod, 
                    arrayTy, 
                    true, 
                    GlobalVariable::InternalLinkage, 
                    ConstantArray::get(arrayTy, keys), 
                    "key");
                GlobalVariable *value1 = new GlobalVariable(
                    *mod, 
                    ArrayType::get(Type::getInt32Ty(context), inst_size), 
                    true, 
                    GlobalVariable::InternalLinkage, 
                    ConstantArray::get(arrayTy, values), 
                    "value");
                
                IRBuilder<> Builder(&*bb1);
                Builder.SetInsertPoint(bb1->getTerminator());

                Constant *map_size = ConstantInt::get(Type::getInt32Ty(context), inst_size);
                Value *key2 = Builder.CreatePointerCast(key1,Type::getInt32PtrTy(context));
                Value *value2 = Builder.CreatePointerCast(value1,Type::getInt32PtrTy(context));
                
                arguments.push_back(map_size);
                arguments.push_back(key2);
                arguments.push_back(value2);

                Builder.CreateCall(update_inst, arguments);

                for (BasicBlock::iterator i3 = bb1->begin(); i3 != bb1->end(); ++i3) {
                    if ((string) i3->getOpcodeName() == "ret") {
                        Builder.SetInsertPoint(&*i3);
                        Builder.CreateCall(print_inst);
                    }
                }
            }

            return false;
        }
    }; 
}

char CountDynamicInstructions::ID = 0;
static RegisterPass<CountDynamicInstructions> X("cse231-cdi", "Count Dynamic Instructions",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
