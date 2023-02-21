#include "codegen_initializer.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

static CGResult<CGExpr>
struct_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	Vec<ir::IRDesignator*>& initializers,
	sema::TypeInstance type,
	LValue lvalue)
{
	auto maybe_llvm_type = get_type(codegen, type);
	if( !maybe_llvm_type.ok() )
		return maybe_llvm_type;
	auto llvm_type = maybe_llvm_type.unwrap();

	for( auto designator : initializers )
	{
		auto member = designator->member;
		auto ir_expr = designator->expr;

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

static CGResult<CGExpr>
struct_initializer(
	CG& codegen, cg::LLVMFnInfo& fn, ir::IRInitializer* ir_initializer, LValue lvalue)
{
	return struct_initializer(
		codegen, fn, *ir_initializer->initializers, ir_initializer->type_instance, lvalue);
}

static CGResult<CGExpr>
enum_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	sema::TypeInstance enum_type,
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
	return struct_initializer(codegen, fn, ir_initializer, member_lvalue);
}

CGResult<CGExpr>
cg::codegen_initializer(
	CG& codegen,
	cg::LLVMFnInfo& fn,
	ir::IRInitializer* ir_initializer,
	std::optional<LValue> maybe_lvalue)
{
	auto lvalue = maybe_lvalue.value();

	auto storage_type = ir_initializer->type_instance.storage_type();
	if( storage_type.is_enum_type() )
		return enum_initializer(
			codegen, fn, ir_initializer, storage_type, ir_initializer->type_instance, lvalue);
	else if( storage_type.is_struct_type() )
		return struct_initializer(codegen, fn, ir_initializer, lvalue);
	// TODO: union and struct.
	// if( !ctype )
	// {
	// 	auto maybe_llvm_type = get_type(codegen, ir_initializer->type_instance);
	// 	if( !maybe_llvm_type.ok() )
	// 		return maybe_llvm_type;
	// 	auto llvm_type = maybe_llvm_type.unwrap();

	// 	for( auto [designator, ir_expr] : *ir_initializer->initializers )
	// 	{
	// 		auto maybe_member = ir_initializer->type_instance.type->get_member(designator);
	// 		auto member = maybe_member.value();

	// 		auto llvm_member_value = codegen.Builder->CreateStructGEP(
	// 			llvm_type, lvalue.address().llvm_pointer(), member.idx);

	// 		auto designator_lvalue =
	// 			LValue(llvm_member_value, llvm_member_value->getType()->getPointerElementType());

	// 		auto rexprr = codegen.codegen_expr(fn, ir_expr, designator_lvalue);

	// 		auto rexpr = rexprr.unwrap();
	// 		if( rexpr.is_empty() )
	// 			continue;

	// 		auto rhs = codegen_operand_expr(codegen, rexpr);

	// 		codegen.Builder->CreateStore(rhs, llvm_member_value);
	// 	}

	// 	return CGExpr();
	// }
	// else
	// {
	// 	auto enum_type = sema::TypeInstance::OfType(ctype);

	// 	auto maybe_llvm_enum_type = get_type(codegen, enum_type);
	// 	if( !maybe_llvm_enum_type.ok() )
	// 		return maybe_llvm_enum_type;
	// 	auto llvm_enum_type = maybe_llvm_enum_type.unwrap();

	// 	bool found = false;
	// 	auto member_type = ir_initializer->type_instance;
	// 	auto maybe_llvm_member_type = get_type(codegen, member_type);
	// 	if( !maybe_llvm_member_type.ok() )
	// 		return maybe_llvm_member_type;
	// 	auto llvm_enum_member_type = maybe_llvm_member_type.unwrap();

	// 	auto nominal = member_type.as_nominal();

	// 	auto llvm_enum_value =
	// 		codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 0);

	// 	llvm::Value* llvm_wanted_value =
	// 		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, nominal.value, true));

	// 	codegen.Builder->CreateStore(llvm_wanted_value, llvm_enum_value);

	// 	auto llvm_enum_union_value =
	// 		codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 1);

	// 	auto llvm_enum_union_casted_value = codegen.Builder->CreateBitCast(
	// 		llvm_enum_union_value, llvm_enum_member_type->getPointerTo());

	// 	for( auto [designator, ir_expr] : *ir_initializer->initializers )
	// 	{
	// 		//
	// 		auto maybe_member = ir_initializer->type_instance.type->get_member(designator);
	// 		auto member = maybe_member.value();

	// 		auto llvm_member_value = codegen.Builder->CreateStructGEP(
	// 			llvm_enum_member_type, llvm_enum_union_casted_value, member.idx);

	// 		auto designator_lvalue =
	// 			LValue(llvm_member_value, llvm_member_value->getType()->getPointerElementType());

	// 		auto rexprr = codegen.codegen_expr(fn, ir_expr, designator_lvalue);

	// 		auto rexpr = rexprr.unwrap();
	// 		if( rexpr.is_empty() )
	// 			continue;

	// 		auto rhs = codegen_operand_expr(codegen, rexpr);

	// 		codegen.Builder->CreateStore(rhs, llvm_member_value);
	// 	}

	// 	return CGExpr();
	// }
}