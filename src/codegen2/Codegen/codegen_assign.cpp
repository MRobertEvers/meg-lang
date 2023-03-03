#include "codegen_assign.h"

#include "../Codegen.h"
#include "cg_division.h"
#include "codegen_binop.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

static ast::BinOp
to_binop(ast::AssignOp op)
{
	switch( op )
	{
	case ast::AssignOp::add:
		return ast::BinOp::plus;
	case ast::AssignOp::sub:
		return ast::BinOp::minus;
	case ast::AssignOp::div:
		return ast::BinOp::slash;
	case ast::AssignOp::mul:
		return ast::BinOp::star;
	default:
		assert(0);
	}
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
trunc_ints(CG& codegen, sema::TypeInstance dest, llvm::Value* rhs)
{
	if( dest.is_pointer_type() )
		return rhs;
	auto size = dest.type->int_width();
	auto other_size = rhs->getType()->getIntegerBitWidth();
	if( size == other_size )
		return rhs;

	if( codegen.sema.types.is_signed_integer_type(dest) )
		return codegen.Builder->CreateSExtOrTrunc(rhs, sizefor(codegen, size));
	else
		return codegen.Builder->CreateZExtOrTrunc(rhs, sizefor(codegen, size));
}

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
	case ast::AssignOp::sub:
	case ast::AssignOp::mul:
	case ast::AssignOp::div:
	{
		auto lhs_value = codegen_operand_expr(codegen, lexpr);
		auto llvm_rval = codegen_arithmetic_binop(
			codegen, //
			SemaTypedInt(ir_assign->lhs->type_instance, lhs_value),
			SemaTypedInt(ir_assign->rhs->type_instance, rhs),
			to_binop(ir_assign->op));

		auto llvm_rval_expr = llvm_rval.unwrap();
		rhs = codegen_operand_expr(codegen, llvm_rval_expr);

		rhs = trunc_ints(codegen, ir_assign->lhs->type_instance, rhs);
		codegen.Builder->CreateStore(rhs, lhs);
	}
	break;
	case ast::AssignOp::assign:
		rhs = trunc_ints(codegen, ir_assign->lhs->type_instance, rhs);
		codegen.Builder->CreateStore(rhs, lhs);
		break;
	}

	return CGExpr();
}