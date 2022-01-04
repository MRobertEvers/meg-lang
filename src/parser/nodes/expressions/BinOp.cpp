#include "BinOp.h"

using namespace llvm;
using namespace nodes;

/// BinaryExprAST - Expression class for a binary operator.
Value*
BinaryOperation::codegen(CodegenContext& codegen)
{
	Value* L = LHS->codegen(codegen);
	Value* R = RHS->codegen(codegen);
	if( !L || !R )
		return nullptr;

	switch( Op )
	{
	case '+':
		return codegen.Builder->CreateAdd(L, R, "addtmp");
	case '-':
		return codegen.Builder->CreateSub(L, R, "subtmp");
	case '*':
		return codegen.Builder->CreateMul(L, R, "multmp");
	case '/':
		return codegen.Builder->CreateSDiv(L, R, "divtmp");
	case '<':
		L = codegen.Builder->CreateICmpULT(L, R, "cmptmp");
		// Convert bool 0/1 to double 0.0 or 1.0
		return L;
	default:
		return nullptr;
	}
}
