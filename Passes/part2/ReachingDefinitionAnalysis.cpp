#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "231DFA.h"

#include <map>
#include <set>
#include <vector>
#include <algorithm>

using namespace llvm;
using namespace std;

class ReachingInfo : public Info {
	private:
		set<unsigned> Info_Set;

	public:
		ReachingInfo() {}
		ReachingInfo(set<unsigned> s) { Info_Set = s; }
		void setInfoSet(set<unsigned> s) { Info_Set = s; }

		void print() {
			for (auto i = Info_Set.begin(); i != Info_Set.end(); i++) {
				errs() << *i << "|";
			}
			errs() << "\n";
		}

		set<unsigned> getInfoSet() { return Info_Set; }

		static bool equals(ReachingInfo* Info1, ReachingInfo* Info2) {
			return Info1->getInfoSet() == Info2->getInfoSet();
		}

		static ReachingInfo* join(ReachingInfo* Info1, ReachingInfo* Info2, ReachingInfo* result) {
			set<unsigned> Info1_Set = Info1->getInfoSet();
			set<unsigned> Info2_Set = Info2->getInfoSet();

			for (auto i = Info2_Set.begin(); i != Info2_Set.end(); i++) {
				Info1_Set.insert(*i);
			}
			result->setInfoSet(Info1_Set);

			return result;
		}
};


class ReachingDefinitionAnalysis : public DataFlowAnalysis<ReachingInfo, true> {
	private:
		typedef pair<unsigned, unsigned> Edge;
		map<string, int> OpToCategory = {
										 {"alloca", 1}, {"fcmp", 1}, {"getelementptr", 1},
										 {"load", 1}, {"select", 1}, {"icmp", 1}, 
										 {"br", 2}, {"store", 2}, {"switch", 2},  
										 {"phi", 3}
										};
	
	public:
		ReachingDefinitionAnalysis(ReachingInfo & bottom, 
								   ReachingInfo & Ini_State) : 
								   DataFlowAnalysis(bottom, Ini_State) {}

		void flowfunction(Instruction* I, 
						  std::vector<unsigned> & IncomingEdges,
						  std::vector<unsigned> & OutgoingEdges,
						  std::vector<ReachingInfo *> & Infos) {
			
			// Instruction Index
			unsigned Instr_Idx = getInstrToIndex()[I];
			map<Edge, ReachingInfo*> EdgeToInfo = getEdgeToInfo();
			ReachingInfo* Out_Info = new ReachingInfo();

			// Join all incoming infos
			for (unsigned i = 0; i < IncomingEdges.size(); i++) {
				Edge In_Edge = make_pair(IncomingEdges[i], Instr_Idx);
				ReachingInfo* In_Info = EdgeToInfo[In_Edge];		
				ReachingInfo::join(Out_Info, 
								   In_Info, 
								   Out_Info);
			}
			//	Category
			string op_name = I->getOpcodeName();
			int op_category = OpToCategory[op_name];
			int category = OpToCategory.count(op_name) ? op_category : 2;
			category = I ->isBinaryOp() ? 1 : category;

			// Join Index of 1/3
			// Category 1
			if (category == 1) {
				set<unsigned> Idx_Set = {Instr_Idx};
				ReachingInfo::join(Out_Info, 
								   new ReachingInfo(Idx_Set), 
								   Out_Info);
			}
			// Category 3
			else if (category == 3) {
				Instruction* NonPHI1 = I->getParent()->getFirstNonPHI();
				unsigned NonPHI1_Idx = getInstrToIndex()[NonPHI1];
				set<unsigned> Idx_Set;

				for (unsigned i = Instr_Idx; i < NonPHI1_Idx; i++) {
					Idx_Set.insert(i);
				}

				ReachingInfo::join(Out_Info, 
								   new ReachingInfo(Idx_Set), 
								   Out_Info);
			}

			// Distribute result to outgoing edges
			for (unsigned i = 0; i < OutgoingEdges.size(); i++)
				Infos.push_back(Out_Info);
		}
};



struct ReachingDefinitionAnalysisPass : public FunctionPass {

    static char ID;

    ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

    bool runOnFunction(Function &F) override {

        ReachingInfo bottom;
        ReachingInfo Ini_State;
        ReachingDefinitionAnalysis RDA(bottom, Ini_State);

        RDA.runWorklistAlgorithm(&F);
        RDA.print();

        return false;
    }
}; // end of struct ReachingDefinitionAnalysisPass

char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "ReachingDefinitionAnalysis",
                             false ,
                             false );
