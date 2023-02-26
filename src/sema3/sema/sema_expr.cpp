#include "sema_expr.h"

#include "../sema_expected.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_expr(Sema& sema, ast::AstNode* ast)
{
	//
	auto ast_expr = expected(ast, ast::as_expr);
	if( !ast_expr.ok() )
		return ast_expr;
	auto expr_node = ast_expr.unwrap();

	switch( expr_node.expr->type )
	{
	case ast::NodeType::NumberLiteral:
	{
		auto ir_const_int =
			sema.builder().create_const_int(expr_node.expr->data.number_literal.literal);
		return ir::ActionResult(
			ir::RValue(ir_const_int, ir::TypeInstance::OfType(sema.types().i32_type())));
	}
	case ast::NodeType::Expr:
		return sema_expr(sema, expr_node.expr);
	default:
		assert(0 && "NodeType expr not supported.");
	}

	return ir::ActionResult();
}