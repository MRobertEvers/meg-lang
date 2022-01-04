#include "Return.h"

using namespace llvm;
using namespace nodes;

void
Return::codegen(CodegenContext& codegen)
{
	auto RetVal = ReturnExpr->codegen(codegen);
	// Finish off the function.
	codegen.Builder->CreateRet(RetVal);
}