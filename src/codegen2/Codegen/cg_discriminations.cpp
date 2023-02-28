#include "cg_discriminations.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

void
cg::cg_discriminations(CG& codegen, CGExpr& discriminating_expr, Vec<ir::IRParam*>& discriminations)
{
	int ind = 0;
	for( auto param : discriminations )
	{
		auto ir_value_decl = param->data.value_decl;
		auto ir_type_decl = param->data.value_decl->type_decl;
		auto llvm_type = get_type(codegen, ir_type_decl).unwrap();

		auto name = ir_value_decl->name;

		auto enum_value = discriminating_expr.get_discrimination(ind).address();
		auto llvm_enum_value = enum_value.llvm_pointer();
		auto llvm_enum_type = enum_value.llvm_allocated_type();
		auto llvm_member_value_ptr =
			codegen.Builder->CreateStructGEP(llvm_enum_type, llvm_enum_value, 1);

		auto llvm_member_value =
			codegen.Builder->CreateBitCast(llvm_member_value_ptr, llvm_type->getPointerTo());

		auto lval = LValue(llvm_member_value, llvm_type);
		// codegen.values.insert_or_assign(name.id().index(), lval);

		ind++;
	}
}
