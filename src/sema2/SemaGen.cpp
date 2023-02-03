

#include "SemaGen.h"

#include "ast2/AstCasts.h"

using namespace sema;
using namespace ir;
using namespace ast;

SemaResult<IRModule*>
sema::sema_module(Sema2& sema, AstNode* node)
{
	auto result = expected(node, ast::as_module);
	if( !result.ok() )
		return result;

	auto mod = node->data.mod;

	auto stmts = sema.create_tlslist();

	for( auto statement : mod.statements )
	{
		auto statement_result = sema_tls(sema, statement);
		if( !statement_result.ok() )
			return statement_result;

		stmts->push_back(statement_result.unwrap());
	}

	return sema.Module(node, stmts);
}

SemaResult<IRTopLevelStmt*>
sema::sema_tls(Sema2& sema, AstNode* ast)
{
	return SemaError("Not Implemented.");
}