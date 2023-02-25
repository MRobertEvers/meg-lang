#include "sema_module.h"

#include "../SemaLookup.h"
#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "sema_tls.h"

using namespace sema;

SemaResult<ir::BasicBlock*>
sema::sema_module(Sema& sema, ast::AstNode* node)
{
	auto result = expected(node, ast::as_module);
	if( !result.ok() )
		return result;

	auto mod = node->data.mod;

	for( auto statement : mod.statements )
	{
		auto statement_result = sema_tls(sema, statement);
		if( !statement_result.ok() )
			return statement_result;
	}

	return sema.builder().__root();
}