#include "cg_access.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

CGResult<CGExpr>
cg::cg_access(CG& codegen, LLVMAddress address, sema::MemberTypeInstance const& member)
{
	auto llvm_expr_value = address.llvm_pointer();
	auto llvm_expr_type = address.llvm_allocated_type();

	auto expr_ty = member.type;

	auto llvm_member_tyr = get_type(codegen, expr_ty);
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