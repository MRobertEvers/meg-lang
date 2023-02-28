#include "codegen_deref.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_deref(CG& codegen, cg::LLVMFnInfo& fn, ir::IRDeref* ir_addrof)
{
	auto expr_result = codegen.codegen_expr(fn, ir_addrof->expr);
	if( !expr_result.ok() )
		return expr_result;
	auto expr = expr_result.unwrap();

	// We can't just take the address type since the allocated variable might actually be a pointer,
	// so the address's allocated type is a pointer and not the deref type.

	// auto target_expr = codegen_operand_expr(codegen, expr);

	auto llvm_allocated_type = get_type(codegen, ir_addrof->type_instance).unwrap();

	auto address = expr.address();
	// TODO: Expr should return l or rvalue. With the type.
	auto llvm_value =
		codegen.Builder->CreateLoad(address.llvm_allocated_type(), address.llvm_pointer());

	return CGExpr::MakeAddress(LLVMAddress(llvm_value, llvm_allocated_type));
}