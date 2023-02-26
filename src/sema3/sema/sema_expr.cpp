#include "sema_expr.h"

#include "../sema_expected.h"
#include "sema_call.h"
#include "sema_id.h"

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
			ir::Action(ir_const_int, ir::TypeInstance::OfType(sema.types().i32_type())));
	}
	case ast::NodeType::FnCall:
		return sema_call(sema, expr_node.expr);
	case ast::NodeType::Id:
		return sema_id(sema, expr_node.expr);
	case ast::NodeType::Expr:
		return sema_expr(sema, expr_node.expr);
	case ast::NodeType::StringLiteral:
	{
		auto ir_string_lit =
			sema.builder().create_string_literal(*expr_node.expr->data.string_literal.literal);
		return ir::ActionResult(
			ir::Action(ir_string_lit, ir::TypeInstance::PointerTo(sema.types().i8_type(), 1)));
	}
	case ast::NodeType::Empty:
		return ir::ActionResult();
	default:
		assert(0 && "NodeType expr not supported.");
	}

	return ir::ActionResult();
}