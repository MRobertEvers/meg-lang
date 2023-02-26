#include "sema_type_decl.h"

#include "../sema_expected.h"
#include "idname.h"

using namespace sema;
using namespace ir;

SemaResult<TypeInstance>
sema::sema_type_decl(Sema& sema, ast::AstNode* ast)
{
	auto ast_type_decl = expected(ast, ast::as_type_decl);
	if( !ast_type_decl.ok() )
		return ast_type_decl;
	auto type_decl_node = ast_type_decl.unwrap();

	auto qn_name = idname(*type_decl_node.name);
	NameLookupResult lu_result = sema.names().lookup_fqn(qn_name);
	if( !lu_result.is_found() || !lu_result.result().name().is_type() )
		return SemaError("Missing type!");

	auto base_type = lu_result.result().name().type();

	auto type = TypeInstance::PointerTo(base_type.type, type_decl_node.indirection_level);

	return type;
}