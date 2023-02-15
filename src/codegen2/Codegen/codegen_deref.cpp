#include "codegen_deref.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_deref(CG& codegen, cg::LLVMFnInfo& fn, ir::IRDeref* ir_addrof)
{
	auto exprr = codegen.codegen_expr(fn, ir_addrof->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	auto llvm_expr = expr.as_value();
	// TODO: Expr should return l or rvalue. With the type.
	auto llvm_value =
		codegen.Builder->CreateLoad(llvm_expr->getType()->getPointerElementType(), llvm_expr);

	return CGExpr(llvm_value);
}