#include "sema_binop.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_expr.h"

using namespace sema;

// static TypeInstance
// binop_result_type(Sema2& sema, ast::BinOp op, ir::IRExpr* lhs, ir::IRExpr* rhs)
// {
// 	switch( op )
// 	{
// 	case ast::BinOp::plus:
// 	case ast::BinOp::star:
// 	case ast::BinOp::minus:
// 	case ast::BinOp::slash:
// 		return lhs->type_instance;
// 	case ast::BinOp::gt:
// 	case ast::BinOp::gte:
// 	case ast::BinOp::lt:
// 	case ast::BinOp::lte:
// 	case ast::BinOp::and_op:
// 	case ast::BinOp::or_op:
// 	case ast::BinOp::cmp:
// 	case ast::BinOp::ne:
// 		return sema.types.BoolType();
// 	case ast::BinOp::bad:
// 		assert(0);
// 		break;
// 	}
// }

SemaResult<ir::ActionResult>
sema::sema_binop(Sema& sema, ast::AstNode* ast)
{
	auto ast_binop = expected(ast, ast::as_binop);
	if( !ast_binop.ok() )
		return ast_binop;
	auto binop_node = ast_binop.unwrap();

	auto lhs_result = sema_expr(sema, binop_node.left);
	if( !lhs_result.ok() )
		return lhs_result;
	auto lhs = lhs_result.unwrap();

	auto rhs_result = sema_expr(sema, binop_node.right);
	if( !rhs_result.ok() )
		return rhs_result;
	auto rhs = rhs_result.unwrap();

	auto type = binop_result_type(sema, binop.op, lhs, rhs);

	// sema.builder()
	// 	.create_binop()

	// if( !compatible_binop_int_types(sema, lhs->type_instance, rhs->type_instance) &&
	// 	!sema.types.equal_types(lhs->type_instance, rhs->type_instance) )
	// {
	// 	return SemaError(
	// 		"Mismatched types: " + sema::to_string(lhs->type_instance) +
	// 		" != " + sema::to_string(rhs->type_instance));
	// }

	return sema.BinOp(ast, binop.op, lhs, rhs, type);
}