#include "codegen_addressof.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_addressof(CG& codegen, cg::LLVMFnInfo& fn, ir::IRAddressOf* ir_addrof)
{
	auto exprr = codegen.codegen_expr(fn, ir_addrof->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	auto llvm_expr = expr.as_value();

	return CGExpr(llvm_expr);
}