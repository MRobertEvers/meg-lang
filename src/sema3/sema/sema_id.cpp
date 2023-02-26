#include "sema_id.h"

#include "../sema_expected.h"
#include "idname.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_id(Sema& sema, ast::AstNode* ast)
{
	auto ast_id = expected(ast, ast::as_id);
	if( !ast_id.ok() )
		return ast_id;
	auto id_node = ast_id.unwrap();

	auto id_name = idname(id_node);
	ir::NameLookupResult lu_result = sema.names().lookup(id_name);
	if( !lu_result.is_found() )
		return SemaError("Use of undeclared identifier '" + id_name.to_string() + "'.");

	auto name = lu_result.result();
	ir::VarRef* ref = sema.builder().create_var_ref(name.id());

	return ir::ActionResult(ir::Action(ref, name.name().type()));
}