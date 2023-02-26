#include "sema_namespace.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "idname.h"
#include "sema_tls.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_namespace(Sema& sema, ast::AstNode* node)
{
	auto ast_namespace = expected(node, ast::as_namespace);
	if( !ast_namespace.ok() )
		return ast_namespace;
	auto namespace_node = ast_namespace.unwrap();

	auto qualified_name = idname(namespace_node.namespace_name->data.id);
	ir::NameRef nspace =
		sema.names().add_name(ir::Name(qualified_name.part(0), sema.names().current().id()));

	sema.names().push_scope(nspace);

	auto mod = node->data.namespace_node;

	for( auto statement : mod.statements )
	{
		auto statement_result = sema_tls(sema, statement);
		if( !statement_result.ok() )
			return statement_result;
	}

	sema.names().pop_scope();

	return ir::ActionResult();
}