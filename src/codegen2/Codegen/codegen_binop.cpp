#include "codegen_binop.h"

#include "../Codegen.h"
#include "CGNotImpl.h"
#include "cg_division.h"
#include "operand.h"

using namespace cg;

// Clang does widening then truncating.
// static

enum class WidenSign
{
	Signed,
	Unsigned,
};
WidenSign
signof(CG& codegen, sema::TypeInstance type)
{
	if( codegen.sema.types.is_signed_integer_type(type) )
		return WidenSign::Signed;
	else
		return WidenSign::Unsigned;
}

static llvm::Type*
sizefor(CG& codegen, int size)
{
	switch( size )
	{
	case 8:
		return llvm::Type::getInt8Ty(*codegen.Context);
	case 16:
		return llvm::Type::getInt16Ty(*codegen.Context);
	case 32:
		return llvm::Type::getInt32Ty(*codegen.Context);
	case 64:
		return llvm::Type::getInt64Ty(*codegen.Context);
	default:
		assert(0);
	}
}

static llvm::Value*
widen(CG& codegen, llvm::Value* value, WidenSign sign, int size)
{
	switch( sign )
	{
	case WidenSign::Signed:
		// S is for Signed-Extend
		return codegen.Builder->CreateSExt(value, sizefor(codegen, size));
	case WidenSign::Unsigned:
		// Z is for Zero-Extend
		return codegen.Builder->CreateZExt(value, sizefor(codegen, size));
	}
}

CGExpr
codegen_numeric_binop(CG& codegen, SemaTypedInt lhs, SemaTypedInt rhs, ast::BinOp op)
{
	llvm::Type* llvm_lhs_type = lhs.value->getType();
	llvm::Type* llvm_rhs_type = rhs.value->getType();
	auto left_width = llvm_lhs_type->getIntegerBitWidth();
	auto right_width = llvm_rhs_type->getIntegerBitWidth();
	auto max_width = left_width > right_width ? left_width : right_width;

	lhs.value = widen(codegen, lhs.value, signof(codegen, lhs.type), max_width);
	rhs.value = widen(codegen, rhs.value, signof(codegen, lhs.type), max_width);

	switch( op )
	{
	case ast::BinOp::plus:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateAdd(lhs.value, rhs.value)));
	case ast::BinOp::minus:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateSub(lhs.value, rhs.value)));
	case ast::BinOp::star:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateMul(lhs.value, rhs.value)));
	case ast::BinOp::slash:
	{
		auto llvm_result = cg_division(codegen, lhs.value, rhs.value, lhs.type);
		return CGExpr::MakeRValue(RValue(llvm_result));
	}
	default:
		assert(0);
	}
}

CGExpr
codegen_pointer_binop(CG& codegen, SemaTypedInt lhs, SemaTypedInt rhs, ast::BinOp op)
{
	SemaTypedInt pointer = lhs.value->getType()->isPointerTy() ? lhs : rhs;
	SemaTypedInt numeric = lhs.value->getType()->isPointerTy() ? rhs : lhs;

	switch( op )
	{
	case ast::BinOp::plus:
	{
		llvm::Value* llvm_gep_arith = codegen.Builder->CreateGEP(
			lhs.value->getType()->getPointerElementType(), pointer.value, numeric.value);
		return CGExpr::MakeRValue(RValue(llvm_gep_arith));
	}
	case ast::BinOp::minus:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateSub(lhs.value, rhs.value)));
	default:
		assert(0);
	}
}

CGResult<CGExpr>
cg::codegen_arithmetic_binop(CG& codegen, SemaTypedInt lhs, SemaTypedInt rhs, ast::BinOp op)
{
	llvm::Type* llvm_lhs_type = lhs.value->getType();
	llvm::Type* llvm_rhs_type = rhs.value->getType();

	if( llvm_lhs_type->isPointerTy() || llvm_rhs_type->isPointerTy() )
		return codegen_pointer_binop(codegen, lhs, rhs, op);
	else
		return codegen_numeric_binop(codegen, lhs, rhs, op);
}

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

	// assert(llvm_lhs->getType()->isIntegerTy() && llvm_rhs->getType()->isIntegerTy());

	switch( ir_binop->op )
	{
	case ast::BinOp::plus:
	case ast::BinOp::minus:
	case ast::BinOp::star:
	case ast::BinOp::slash:
		return codegen_arithmetic_binop(
			codegen,
			SemaTypedInt(ir_binop->lhs->type_instance, llvm_lhs),
			SemaTypedInt(ir_binop->rhs->type_instance, llvm_rhs),
			ir_binop->op);
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

		// TODO: These two are different.
	case ast::BinOp::and_op:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateAnd(llvm_lhs, llvm_rhs)));
	case ast::BinOp::or_op:
		return CGExpr::MakeRValue(RValue(codegen.Builder->CreateOr(llvm_lhs, llvm_rhs)));
	default:
		return NotImpl();
	}
}