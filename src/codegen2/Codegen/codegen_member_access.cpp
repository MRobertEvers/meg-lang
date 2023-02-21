#include "codegen_member_access.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_indirect_member_access(
	CG& codegen, cg::LLVMFnInfo& fn, ir::IRIndirectMemberAccess* ir_ma)
{
	auto exprr = codegen.codegen_expr(fn, ir_ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto llvm_expr_ptr_value = expr.address().llvm_pointer();
	auto llvm_expr_ptr_type = expr.address().llvm_allocated_type();

	// auto struct_ty_name = expr_ty.type->get_name();
	auto expr_ty = ir_ma->expr->type_instance;
	assert(
		(expr_ty.type->is_struct_type() || expr_ty.type->is_union_type()) &&
		expr_ty.indirection_level == 1);

	// TODO: Don't do this
	auto llvm_expr_type = llvm_expr_ptr_type->getPointerElementType();

	auto member_name = *ir_ma->member_name;
	auto maybe_member = expr_ty.type->get_member(member_name);
	assert(maybe_member.has_value());

	auto member = maybe_member.value();
	auto llvm_member_tyr = get_type(codegen, member.type);
	if( !llvm_member_tyr.ok() )
		return llvm_member_tyr;
	auto llvm_member_type = llvm_member_tyr.unwrap();

	auto llvm_expr_value = codegen.Builder->CreateLoad(llvm_expr_ptr_type, llvm_expr_ptr_value);
	if( ir_ma->type_instance.is_struct_type() )
	{
		auto llvm_member_value =
			codegen.Builder->CreateStructGEP(llvm_expr_type, llvm_expr_value, member.idx);

		return CGExpr::MakeAddress(LLVMAddress(llvm_member_value, llvm_member_type));
	}
	else
	{
		// TODO: Opaque pointer
		auto llvm_member_value =
			codegen.Builder->CreateBitCast(llvm_expr_value, llvm_member_type->getPointerTo());

		return CGExpr::MakeAddress(LLVMAddress(llvm_member_value, llvm_member_type));
	}
}

CGResult<CGExpr>
cg::codegen_member_access(CG& codegen, cg::LLVMFnInfo& fn, ir::IRMemberAccess* ir_ma)
{
	auto exprr = codegen.codegen_expr(fn, ir_ma->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto llvm_expr_value = expr.address().llvm_pointer();
	auto llvm_expr_type = expr.address().llvm_allocated_type();

	auto expr_ty = ir_ma->expr->type_instance;
	assert(
		(expr_ty.type->is_struct_type() || expr_ty.type->is_union_type()) &&
		expr_ty.indirection_level == 0);

	auto member_name = *ir_ma->member_name;
	auto maybe_member = expr_ty.type->get_member(member_name);
	assert(maybe_member.has_value());

	auto member = maybe_member.value();
	auto llvm_member_tyr = get_type(codegen, member.type);
	if( !llvm_member_tyr.ok() )
		return llvm_member_tyr;
	auto llvm_member_type = llvm_member_tyr.unwrap();

	if( expr_ty.is_struct_type() )
	{
		auto llvm_member_value =
			codegen.Builder->CreateStructGEP(llvm_expr_type, llvm_expr_value, member.idx);

		return CGExpr::MakeAddress(LLVMAddress(llvm_member_value, llvm_member_type));
	}
	else
	{
		auto llvm_member_value =
			codegen.Builder->CreateBitCast(llvm_expr_value, llvm_member_type->getPointerTo());
		return CGExpr::MakeAddress(LLVMAddress(llvm_member_value, llvm_member_type));
	}
}