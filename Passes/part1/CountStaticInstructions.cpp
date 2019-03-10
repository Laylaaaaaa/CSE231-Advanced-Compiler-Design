#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"

#include <map>

using namespace std;
using namespace llvm;

namespace {
    struct CountStaticInstructions : public FunctionPass {
        static char ID;
        CountStaticInstructions() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            map<string, int> sta_inst_map;

            for (inst_iterator i1 = inst_begin(F); i1 != inst_end(F); ++i1) {
                string inst_name = i1->getOpcodeName();
                sta_inst_map[inst_name]++;
            }

            for (map<string, int>::iterator i1 = sta_inst_map.begin(); i1 != sta_inst_map.end(); ++i1) {
                errs() << i1->first << '\t' << i1->second << '\n';
            }

            return false;
        }
    }; 
}  

char CountStaticInstructions::ID = 0;
static RegisterPass<CountStaticInstructions> X("cse231-csi", "Count Static Instructions",
                             false,
                             false);
