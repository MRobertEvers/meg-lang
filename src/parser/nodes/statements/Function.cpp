#include "Function.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

using namespace llvm;
using namespace nodes;

void
Function::codegen()
{
	// Transfer ownership of the prototype to the FunctionProtos map, but keep a
	// reference to it for use below.
	llvm::Function* TheFunction = Proto->codegen();
	if( !TheFunction )
		return nullptr;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
	Builder->SetInsertPoint(BB);

	Body->codegen();
}