#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"


using namespace llvm;

namespace {
struct count_bb : public ModulePass {
	static char ID;
	Function *hook;
	unsigned long block_counter = 0;
	count_bb() : ModulePass(ID) {}

	virtual bool runOnBasicBlock(Function::iterator &BB);
	virtual bool runOnModule(Module &M);
};

char count_bb::ID = 0;

bool count_bb::runOnModule(Module &M)
{
	Constant *hookFunc;
	hookFunc = M.getOrInsertFunction("countBlocks", Type::getVoidTy(M.getContext()), 
		Type::getInt64Ty(M.getContext()), Type::getInt64Ty(M.getContext()), NULL);
	hook = cast<Function>(hookFunc);
	errs() << "Found the counting function\n";

	for (Module::iterator F = M.begin(), E = M.end(); F != E; ++F)
	{
		for (Function::iterator BB = F->begin(), E = F->end(); BB != E; ++BB)
		{
			if (!F->getName().equals(StringRef("countBlocks")))
			{
				count_bb::runOnBasicBlock(BB);
				block_counter++;
			}
			if (F->getName().equals(StringRef("main")))
			{
				TerminatorInst *term = BB->getTerminator();
				if (term->getNumSuccessors() == 0)
				{
					IRBuilder<> Builder((Instruction *) term);
					Value *args[] = {ConstantInt::get(Type::getInt64Ty(term->getContext()), -1UL), ConstantInt::get(Type::getInt64Ty(term->getContext()), 0UL)};
					Instruction *newInst = CallInst::Create(hook, args);
					Builder.Insert(newInst);
				}
			}
		}
	}
	errs() << "done\n";
	return false;
}

bool count_bb::runOnBasicBlock(Function::iterator &BB)
{
	errs() << "Adding new counter for basic block" << block_counter << "\n";
	Instruction *CI = dyn_cast<Instruction>(BB->getTerminator());
	IRBuilder<> Builder((Instruction *) CI);
	Value *args[] = {ConstantInt::get(Type::getInt64Ty(CI->getContext()),
		block_counter), ConstantInt::get(Type::getInt64Ty(CI->getContext()),
		(unsigned long) BB->size())};
	Instruction *newInst = CallInst::Create(hook, args);
	Builder.Insert(newInst);
	return true;
}

} //end namespace

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
/*
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
*/
static RegisterPass<count_bb> X("countBB", "counting basic block executions", false, false);