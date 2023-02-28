#include "sema_namespace.h"

#include "../SemaGen.h"
#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "idname.h"

using namespace sema;

SemaResult<ir::IRNamespace*>
sema::sema_namespace(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(node, ast::as_namespace);
	if( !result.ok() )
		return result;

	auto nspace = result.unwrap();

	// TODO: Simple name?
	QualifiedName name = idname(nspace.namespace_name->data.id);
	std::string name_str = name.part(0);

	sema::NameRef nspace_ref = sema.add_namespace(name_str);
	sema.push_scope(nspace_ref);

	std::vector<ir::IRTopLevelStmt*> stmts;
	for( auto statement : nspace.statements )
	{
		auto statement_result = sema_tls(sema, statement);
		if( !statement_result.ok() )
			return statement_result;

		stmts.push_back(statement_result.unwrap());
	}

	sema.pop_scope();

	return sema.Namespace(node, stmts);
}
