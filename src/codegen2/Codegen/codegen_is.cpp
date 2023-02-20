#include "codegen_is.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_is(CG& codegen, cg::LLVMFnInfo& fn, ir::IRIs* ir_is)
{
	//
	auto lhsr = codegen.codegen_expr(fn, ir_is->lhs);
	if( !lhsr.ok() )
		return lhsr;
	auto lhs = lhsr.unwrap();

	auto enum_type = ir_is->type_decl->type_instance.type->get_dependent_type();
	assert(enum_type && enum_type->is_enum_type());

	long long wanted_val = -1;
	bool found = false;
	// TODO: Better lookup
	auto test_type = ir_is->type_decl->type_instance.type;
	for( int i = 0; i < enum_type->get_member_count(); i++ )
	{
		auto mem = enum_type->get_member(i);
		auto member_type = mem.type.type;
		if( member_type == test_type )
		{
			wanted_val = mem.idx;
			found = true;
			break;
		}
	}
	assert(found);

	llvm::Value* llvm_wanted_value =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, wanted_val, true));

	auto llvm_member_type_value = codegen.Builder->CreateStructGEP(
		lhs.address().llvm_allocated_type(), lhs.address().llvm_pointer(), 0);
	auto llvm_lhs = codegen.Builder->CreateLoad(
		llvm_member_type_value->getType()->getPointerElementType(), llvm_member_type_value);

	auto result =
		CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpEQ(llvm_lhs, llvm_wanted_value)));

	result.add_discrimination(lhs);
	return result;
}