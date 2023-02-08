#include "codegen_return.h"

#include "../Codegen.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_return(CG& codegen, cg::LLVMFnInfo& fn, ir::IRReturn* ir_return)
{
	auto exprr = codegen.codegen_expr(fn, ir_return->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto llvm_value = expr.as_value();

	if( fn.has_sret_arg() )
	{
		auto sret_arg = fn.sret();
		auto llvm_sret_value = sret_arg.lvalue.value();
		auto llvm_sret_type = sret_arg.lvalue.type();

		// TODO: Compute alignment from member
		auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(
			llvm_sret_type->getPointerElementType());
		auto llvm_align = codegen.Module->getDataLayout().getPrefTypeAlign(
			llvm_sret_type->getPointerElementType());

		codegen.Builder->CreateMemCpy(
			llvm_sret_value, llvm_align, llvm_value, llvm_align, llvm_size);
		codegen.Builder->CreateRetVoid();
	}
	else
	{
		codegen.Builder->CreateRet(llvm_value);
	}

	// if( fn.ret_type == LLVMFnSigRetType::SRet )
	// {
	// 	auto llvm_fn = fn.llvm_fn;
	// 	auto SRet = Function->getArg(0);
	// 	auto Expr = expr.as_value();

	// 	// TODO: Compute alignment from member
	// 	auto Size = codegen.Module->getDataLayout().getTypeAllocSize(
	// 		Expr->getType()->getPointerElementType());
	// 	auto Align = codegen.Module->getDataLayout().getPrefTypeAlign(
	// 		Expr->getType()->getPointerElementType());

	// 	codegen.Builder->CreateMemCpy(SRet, Align, Expr, Align, Size);
	// 	codegen.Builder->CreateRetVoid();
	// }
	// else
	// {
	// 	codegen.Builder->CreateRet(expr.as_value());
	// }

	return CGExpr();
}