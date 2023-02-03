

#include "SemaGen.h"

#include "ast2/AstCasts.h"

using namespace sema;
using namespace ir;
using namespace ast;

SemaResult<IRModule>
sema::sema_module(Sema2& sema, AstNode* node)
{
	// auto result = ast::as_mod(node);
	// if( !result.ok() )
	// 	return result;

	// auto mod = node->data.mod;

	// for( auto statement : mod.statements )
	// {
	// 	auto statement_result = sema_tls(sema, statement);
	// 	// Don't really care about the sema result here.
	// 	if( !statement_result.ok() )
	// 	{
	// 		return statement_result;
	// 	}
	// }

	// return Ok();

	return SemaError("Not Implemented.");
}

SemaResult<IRTopLevelStmt>
sema::sema_tls(Sema2& sema, AstNode* ast)
{
	return SemaError("Not Implemented.");
}