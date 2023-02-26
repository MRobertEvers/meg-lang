#include "sema_return.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_expr.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_return(Sema& sema, ast::AstNode* ast)
{
	auto ast_return = expected(ast, ast::as_fn_return);
	if( !ast_return.ok() )
		return ast_return;
	auto return_node = ast_return.unwrap();

	auto expr_result = sema_expr(sema, return_node.expr);
	if( !expr_result.ok() )
		return expr_result;

	auto expr = expr_result.unwrap();

	if( expr.is_void() )
	{
		sema.builder().create_return(nullptr);
		return ir::ActionResult();
	}
	else
	{
		sema.builder().create_return(expr.action().inst);

		return ir::ActionResult();
	}
}