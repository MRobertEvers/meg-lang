#include "codegen_assign.h"

#include "../Codegen.h"
#include "cg_division.h"
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
	case ast::AssignOp::add:
	{
		auto lhs_value = codegen_operand_expr(codegen, lexpr);
		auto llvm_rval = codegen.Builder->CreateAdd(rhs, lhs_value);
		codegen.Builder->CreateStore(llvm_rval, lhs);
	}
	break;
	case ast::AssignOp::sub:
	{
		auto lhs_value = codegen_operand_expr(codegen, lexpr);
		auto llvm_rval = codegen.Builder->CreateSub(rhs, lhs_value);
		codegen.Builder->CreateStore(llvm_rval, lhs);
	}
	break;
	case ast::AssignOp::mul:
	{
		auto lhs_value = codegen_operand_expr(codegen, lexpr);
		auto llvm_rval = codegen.Builder->CreateMul(rhs, lhs_value);
		codegen.Builder->CreateStore(llvm_rval, lhs);
	}
	break;
	case ast::AssignOp::div:
	{
		// Clang chooses SDiv vs UDiv based on the numerator
		auto lhs_value = codegen_operand_expr(codegen, lexpr);
		auto llvm_rval = cg_division(codegen, lhs_value, rhs, ir_assign->lhs->type_instance);

		codegen.Builder->CreateStore(llvm_rval, lhs);
	}
	break;
	case ast::AssignOp::assign:
		codegen.Builder->CreateStore(rhs, lhs);
		break;
	}

	return CGExpr();
}