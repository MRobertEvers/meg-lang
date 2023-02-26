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

	if( return_node.expr )
	{
		sema.builder().create_return(nullptr);
		return ir::ActionResult();
	}
	else
	{
		auto expr_result = sema_expr(sema, ast);
		if( !expr_result.ok() )
			return expr_result;

		auto expr = expr_result.unwrap();

		sema.builder().create_return(expr.rvalue().inst);

		return ir::ActionResult();
	}
}