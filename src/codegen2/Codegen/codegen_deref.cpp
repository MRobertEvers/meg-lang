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

	auto address = expr.address();
	// TODO: Expr should return l or rvalue. With the type.
	auto llvm_value =
		codegen.Builder->CreateLoad(address.llvm_allocated_type(), address.llvm_pointer());

	return CGExpr::MakeRValue(RValue(llvm_value, address.llvm_allocated_type()));
}