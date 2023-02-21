#include "codegen_initializer.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	std::optional<LValue> maybe_lvalue)
{
	//
	auto lvalue = maybe_lvalue.value();

	auto maybe_llvm_type = get_type(codegen, ir_initializer->type_instance);
	if( !maybe_llvm_type.ok() )
		return maybe_llvm_type;
	auto llvm_type = maybe_llvm_type.unwrap();

	// TODO: Enum types .

	for( auto [designator, ir_expr] : *ir_initializer->initializers )
	{
		//
		auto maybe_member = ir_initializer->type_instance.type->get_member(designator);
		auto member = maybe_member.value();

		auto llvm_member_value = codegen.Builder->CreateStructGEP(
			llvm_type, lvalue.address().llvm_pointer(), member.idx);

		auto designator_lvalue =
			LValue(llvm_member_value, llvm_member_value->getType()->getPointerElementType());

		auto rexprr = codegen.codegen_expr(fn, ir_expr, designator_lvalue);

		auto rexpr = rexprr.unwrap();
		if( rexpr.is_empty() )
			continue;

		auto rhs = codegen_operand_expr(codegen, rexpr);

		codegen.Builder->CreateStore(rhs, llvm_member_value);
	}

	return CGExpr();
}