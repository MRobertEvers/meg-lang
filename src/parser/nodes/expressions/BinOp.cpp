#include "BinOp.h"

using namespace llvm;
using namespace nodes;

/// BinaryExprAST - Expression class for a binary operator.
Value*
BinaryOperation::codegen()
{
	Value* L = LHS->codegen();
	Value* R = RHS->codegen();
	if( !L || !R )
		return nullptr;

	switch( Op )
	{
	case '+':
		return Builder->CreateAdd(L, R, "addtmp");
	case '-':
		return Builder->CreateSub(L, R, "subtmp");
	case '*':
		return Builder->CreateMul(L, R, "multmp");
	case '/':
		return Builder->CreateSDiv(L, R, "divtmp");
	case '<':
		L = Builder->CreateICmpULT(L, R, "cmptmp");
		// Convert bool 0/1 to double 0.0 or 1.0
		return L;
	default:
		return nullptr;
	}
}
