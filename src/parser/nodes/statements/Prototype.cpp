#include "Prototype.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

using namespace llvm;
using namespace nodes;

void
Prototype::codegen(CodegenContext& codegen)
{
	// Make the function type:  double(double,double) etc.
	std::vector<Type*> Doubles(Args.size(), Type::getDoubleTy(*codegen.Context));
	FunctionType* FT = FunctionType::get(Type::getDoubleTy(*codegen.Context), Doubles, false);

	Function* F = Function::Create(FT, Function::ExternalLinkage, Name, codegen.Module.get());

	// Set names for all arguments.
	unsigned Idx = 0;
	for( auto& Arg : F->args() )
		Arg.setName(Args[Idx++]);

	codegen.Functions.insert(std::make_pair(Name, F));
}