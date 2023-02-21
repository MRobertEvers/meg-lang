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

	// TODO: Later, insert a constructor call?
	if( rexpr.is_empty() )
		return CGExpr();

	auto lhs = lexpr.address().llvm_pointer();
	auto rhs = codegen_operand_expr(codegen, rexpr);

	assert(lhs && rhs && "nullptr for assignment!");

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
		codegen.Builder->CreateStore(rhs, lhs);
		break;
	}

	return CGExpr();
}