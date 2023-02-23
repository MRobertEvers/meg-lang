#include "codegen_binop.h"

#include "../Codegen.h"
#include "CGNotImpl.h"
#include "cg_division.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_binop(CG& codegen, cg::LLVMFnInfo& fn, ir::IRBinOp* ir_binop)
{
	auto lhsr = codegen.codegen_expr(fn, ir_binop->lhs);
	if( !lhsr.ok() )
		return lhsr;

	auto rhsr = codegen.codegen_expr(fn, ir_binop->rhs);
	if( !rhsr.ok() )
		return rhsr;

	auto lexpr = lhsr.unwrap();
	auto rexpr = rhsr.unwrap();

	auto llvm_lhs = codegen_operand_expr(codegen, lexpr);
	auto llvm_rhs = codegen_operand_expr(codegen, rexpr);

	assert(llvm_lhs && llvm_rhs && "nullptr for assignment!");

	assert(llvm_lhs->getType()->isIntegerTy() && llvm_rhs->getType()->isIntegerTy());

	switch( ir_binop->op )
	{
	case ast::BinOp::plus:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateAdd(llvm_lhs, llvm_rhs)));
	case ast::BinOp::minus:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateSub(llvm_lhs, llvm_rhs)));
	case ast::BinOp::star:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateMul(llvm_lhs, llvm_rhs)));
	case ast::BinOp::slash:
	{
		auto llvm_result = cg_division(codegen, llvm_lhs, llvm_rhs, ir_binop->lhs->type_instance);
		return CGExpr::MakeRValue(RValue(llvm_result));
	}
	case ast::BinOp::gt:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpSGT(llvm_lhs, llvm_rhs)));
	case ast::BinOp::gte:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpSGE(llvm_lhs, llvm_rhs)));
	case ast::BinOp::lt:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpSLT(llvm_lhs, llvm_rhs)));
	case ast::BinOp::lte:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpSLE(llvm_lhs, llvm_rhs)));
	case ast::BinOp::cmp:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpEQ(llvm_lhs, llvm_rhs)));
	case ast::BinOp::ne:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateICmpNE(llvm_lhs, llvm_rhs)));
	case ast::BinOp::and_op:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateAnd(llvm_lhs, llvm_rhs)));
	case ast::BinOp::or_op:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateOr(llvm_lhs, llvm_rhs)));
	default:
		return NotImpl();
	}
}