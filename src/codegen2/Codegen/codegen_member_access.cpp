#include "codegen_member_access.h"

#include "../Codegen.h"
#include "cg_access.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

static LLVMAddress
dereference(CG& codegen, LLVMAddress const& address)
{
	auto llvm_expr_ptr_value = address.llvm_pointer();
	auto llvm_expr_ptr_type = address.llvm_allocated_type();

	auto llvm_expr_value = codegen.Builder->CreateLoad(llvm_expr_ptr_type, llvm_expr_ptr_value);
	auto llvm_expr_type = llvm_expr_ptr_type->getPointerElementType();

	return LLVMAddress(llvm_expr_value, llvm_expr_type);
}

CGResult<CGExpr>
cg::codegen_indirect_member_access(
	CG& codegen, cg::LLVMFnInfo& fn, ir::IRIndirectMemberAccess* ir_ma)
{
	auto exprr = codegen.codegen_expr(fn, ir_ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	return cg_access(
		codegen,
		dereference(codegen, expr.address()),
		ir_ma->expr->type_instance.Dereference(),
		ir_ma->member);
}

CGResult<CGExpr>
cg::codegen_member_access(CG& codegen, cg::LLVMFnInfo& fn, ir::IRMemberAccess* ir_ma)
{
	auto exprr = codegen.codegen_expr(fn, ir_ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	return cg_access(codegen, expr.address(), ir_ma->expr->type_instance, ir_ma->member);
}