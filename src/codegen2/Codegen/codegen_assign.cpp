#include "codegen_assign.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_assign(CG& codegen, cg::LLVMFnInfo& fn, ir::IRAssign* ir_assign)
{
	auto lhsr = codegen.codegen_expr(fn, ir_assign->lhs);
	if( !lhsr.ok() )
		return lhsr;

	LValue __temp_replace = LValue(lhsr.unwrap().address());

	auto lexpr = lhsr.unwrap();
	auto rhsr = codegen.codegen_expr(fn, ir_assign->rhs, __temp_replace);
	if( !rhsr.ok() )
		return rhsr;

	auto rexpr = rhsr.unwrap();

	// TODO: The RHS might be a Struct type name.
	// E.g.
	//
	// let my_point = Point;
	//
	// In this case, assignment is a no op.
	// TODO: Later, insert a constructor call?
	if( rexpr.is_empty() )
		return CGExpr();

	// auto lhs = codegen_operand_expr(codegen, lexpr);
	auto lhs = lexpr.address().llvm_pointer();
	auto rhs = codegen_operand_expr(codegen, rexpr);

	assert(lhs && rhs && "nullptr for assignment!");

	// TODO: Promote to value really only needs to be done for allocas??
	// Confusing, we'll see where this goes.
	switch( ir_assign->op )
	{
	// case ast::AssignOp::add:
	// {
	// 	auto LValuePromoted = promote_to_value(*this, LValue);
	// 	auto TempRValue = Builder->CreateAdd(RValue, LValuePromoted);
	// 	Builder->CreateStore(TempRValue, LValue);
	// }
	// break;
	// case ast::AssignOp::sub:
	// {
	// 	auto LValuePromoted = promote_to_value(*this, LValue);
	// 	auto TempRValue = Builder->CreateSub(LValuePromoted, RValue);
	// 	Builder->CreateStore(TempRValue, LValue);
	// }
	// break;
	// case ast::AssignOp::mul:
	// {
	// 	auto LValuePromoted = promote_to_value(*this, LValue);
	// 	auto TempRValue = Builder->CreateMul(RValue, LValuePromoted);
	// 	Builder->CreateStore(TempRValue, LValue);
	// }
	// break;
	// case ast::AssignOp::div:
	// {
	// 	auto LValuePromoted = promote_to_value(*this, LValue);
	// 	auto TempRValue = Builder->CreateSDiv(RValue, LValuePromoted);
	// 	Builder->CreateStore(TempRValue, LValue);
	// }
	// break;
	case ast::AssignOp::assign:
		// TODO: Constant expressions return their exact values,
		// All other expressions return a pointer to their result.
		// LValue expressions return a pointer to them, so LValues need
		// to be promoted. Need to create LValue type
		// auto RValuePromoted = !rexpr.literal ? promote_to_value(*this, RValue) : RValue;
		codegen.Builder->CreateStore(rhs, lhs);
		break;
	}

	return CGExpr();
}