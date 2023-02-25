#include "sema_block.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_statement.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_block(Sema& sema, ast::AstNode* ast)
{
	auto ast_block = expected(ast, ast::as_block);
	if( !ast_block.ok() )
		return ast_block;
	auto block_node = ast_block.unwrap();

	for( auto stmt : block_node.statements )
	{
		auto stmt_result = sema_statement(sema, stmt);
		if( !stmt_result.ok() )
			return stmt_result;
	}

	return ir::ActionResult();
}