#include "sema_statement.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_return.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_statement(Sema& sema, ast::AstNode* ast)
{
	auto stmtr = expected(ast, ast::as_stmt);
	if( !stmtr.ok() )
		return stmtr;
	auto stmt = stmtr.unwrap();

	auto stmt_node = stmt.stmt;

	switch( stmt_node->type )
	{
	case ast::NodeType::Return:
		return sema_return(sema, stmt_node);
	}

	return ir::ActionResult();
}