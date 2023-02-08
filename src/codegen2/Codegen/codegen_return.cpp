#include "codegen_return.h"

#include "../Codegen.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_return(CG& codegen, cg::CGFunctionContext& fn, ir::IRReturn* ir_return)
{
	auto exprr = codegen.codegen_expr(fn, ir_return->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	if( fn.ret_type == CGFunctionContext::RetType::SRet )
	{
		auto Function = fn.Fn;
		auto SRet = Function->getArg(0);
		auto Expr = expr.as_value();

		// TODO: Compute alignment from member
		auto Size = codegen.Module->getDataLayout().getTypeAllocSize(
			Expr->getType()->getPointerElementType());
		auto Align = codegen.Module->getDataLayout().getPrefTypeAlign(
			Expr->getType()->getPointerElementType());

		codegen.Builder->CreateMemCpy(SRet, Align, Expr, Align, Size);
		codegen.Builder->CreateRetVoid();
	}
	else
	{
		codegen.Builder->CreateRet(expr.as_value());
	}

	return CGExpr();
}