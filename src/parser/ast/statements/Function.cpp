#include "Function.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

using namespace llvm;
using namespace ast;

void
ast::Function::codegen(CodegenContext& codegen)
{
	// Transfer ownership of the prototype to the FunctionProtos map, but keep a
	// reference to it for use below.
	Proto->codegen(codegen);

	auto TheFunctionIter = codegen.Functions.find(Proto->getName());

	if( TheFunctionIter == codegen.Functions.end() )
		return;

	// Create a new basic block to start insertion into.
	BasicBlock* BB = BasicBlock::Create(*codegen.Context, "entry", TheFunctionIter->second.get());
	codegen.Builder->SetInsertPoint(BB);

	Body->codegen(codegen);
}