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

	auto ctype = ir_initializer->type_instance.type->get_dependent_type();
	if( !ctype )
	{
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
	else
	{
		auto enum_type = sema::TypeInstance::OfType(ctype);

		auto maybe_llvm_enum_type = get_type(codegen, enum_type);
		if( !maybe_llvm_enum_type.ok() )
			return maybe_llvm_enum_type;
		auto llvm_enum_type = maybe_llvm_enum_type.unwrap();

		long long enum_value = -1;
		bool found = false;
		// TODO: Better lookup
		auto test_type = ir_initializer->type_instance.type;
		llvm::Type* llvm_enum_member_type = nullptr;
		for( int i = 0; i < ctype->get_member_count(); i++ )
		{
			auto mem = ctype->get_member(i);
			auto member_type = mem.type.type;
			if( member_type == test_type )
			{
				auto maybe_llvm_member_typer =
					get_type(codegen, sema::TypeInstance::OfType(member_type));
				if( !maybe_llvm_member_typer.ok() )
					return maybe_llvm_member_typer;

				llvm_enum_member_type = maybe_llvm_member_typer.unwrap();

				enum_value = mem.idx;
				found = true;
				break;
			}
		}
		assert(found);

		auto llvm_enum_value =
			codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 0);

		llvm::Value* llvm_wanted_value =
			llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, enum_value, true));

		codegen.Builder->CreateStore(llvm_wanted_value, llvm_enum_value);

		auto llvm_enum_union_value =
			codegen.Builder->CreateStructGEP(llvm_enum_type, lvalue.address().llvm_pointer(), 1);

		auto llvm_enum_union_casted_value = codegen.Builder->CreateBitCast(
			llvm_enum_union_value, llvm_enum_member_type->getPointerTo());

		for( auto [designator, ir_expr] : *ir_initializer->initializers )
		{
			//
			auto maybe_member = ir_initializer->type_instance.type->get_member(designator);
			auto member = maybe_member.value();

			auto llvm_member_value = codegen.Builder->CreateStructGEP(
				llvm_enum_member_type, llvm_enum_union_casted_value, member.idx);

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
}