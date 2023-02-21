#include "codegen_initializer.h"

#include "../Codegen.h"
#include "cg_access.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

static CGResult<CGExpr>
struct_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	sema::TypeInstance const& struct_type,
	Vec<ir::IRDesignator*>& initializers,
	LValue lvalue)
{
	for( auto designator : initializers )
	{
		auto member = designator->member;
		auto ir_expr = designator->expr;

		auto designator_address =
			cg_access(codegen, lvalue.address(), struct_type, member).unwrap().address();

		auto designator_lvalue =
			LValue(designator_address.llvm_pointer(), designator_address.llvm_allocated_type());

		auto rexprr = codegen.codegen_expr(fn, ir_expr, designator_lvalue);
		auto rexpr = rexprr.unwrap();
		if( rexpr.is_empty() )
			continue;

		auto rhs = codegen_operand_expr(codegen, rexpr);

		codegen.Builder->CreateStore(rhs, designator_lvalue.address().llvm_pointer());
	}

	return CGExpr();
}

static CGResult<CGExpr>
enum_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	sema::TypeInstance member_type,
	LValue lvalue)
{
	auto llvm_enum_type = lvalue.address().llvm_allocated_type();

	auto maybe_llvm_member_type = get_type(codegen, member_type);
	if( !maybe_llvm_member_type.ok() )
		return maybe_llvm_member_type;
	auto llvm_enum_member_type = maybe_llvm_member_type.unwrap();

	auto nominal = member_type.as_nominal();

	auto llvm_enum_nominal_value =
		codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 0);

	llvm::Value* llvm_nominal_value =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, nominal.value, true));

	codegen.Builder->CreateStore(llvm_nominal_value, llvm_enum_nominal_value);

	auto llvm_enum_union_value =
		codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 1);

	auto llvm_enum_union_casted_value = codegen.Builder->CreateBitCast(
		llvm_enum_union_value, llvm_enum_member_type->getPointerTo());

	auto member_lvalue = LValue(llvm_enum_union_casted_value, llvm_enum_member_type);
	return struct_initializer(
		codegen, fn, ir_initializer->type_instance, *ir_initializer->initializers, member_lvalue);
}

static CGResult<CGExpr>
union_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	sema::TypeInstance member_type,
	LValue lvalue)
{
	auto maybe_union_type = get_type(codegen, member_type);
	if( !maybe_union_type.ok() )
		return maybe_union_type;
	auto llvm_union_type = maybe_union_type.unwrap();

	auto llvm_union_value = codegen.Builder->CreateBitCast(
		lvalue.address().llvm_pointer(), llvm_union_type->getPointerTo());

	auto bitcasted_lvalue = LValue(llvm_union_value, llvm_union_type);
	return struct_initializer(
		codegen,
		fn,
		ir_initializer->type_instance,
		*ir_initializer->initializers,
		bitcasted_lvalue);
}

CGResult<CGExpr>
cg::codegen_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	std::optional<LValue> maybe_lvalue)
{
	auto lvalue = maybe_lvalue.value();
	// TODO: Allocate if not provided?

	auto storage_type = ir_initializer->type_instance.storage_type();
	if( storage_type.is_enum_type() )
		return enum_initializer(codegen, fn, ir_initializer, ir_initializer->type_instance, lvalue);
	else if( storage_type.is_struct_type() )
		return struct_initializer(
			codegen, fn, ir_initializer->type_instance, *ir_initializer->initializers, lvalue);
	else if( storage_type.is_union_type() )
		return union_initializer(
			codegen, fn, ir_initializer, ir_initializer->type_instance, lvalue);
	else
		return CGExpr();
}