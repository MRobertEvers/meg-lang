#include "sema_let.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "idname.h"
#include "ir/Name.h"
#include "sema_expr.h"
#include "sema_statement.h"
#include "sema_type_decl.h"

using namespace sema;

SemaResult<ir::ActionResult>
sema::sema_let(Sema& sema, ast::AstNode* ast)
{
	auto ast_let = expected(ast, ast::as_let);
	if( !ast_let.ok() )
		return ast_let;
	auto let_node = ast_let.unwrap();

	auto ast_id = expected(let_node.identifier, ast::as_id);
	if( !ast_id.ok() )
		return ast_id;
	auto id_node = ast_id.unwrap();

	// TODO: Assert simple name
	auto var_name = idname(id_node);

	NameLookupResult lu_result = sema.names().lookup(var_name);
	if( lu_result.is_found() )
		return SemaError("Redefinition of " + var_name.to_string());

	auto type_decl_result = sema_type_decl(sema, let_node.type_declarator);
	if( !type_decl_result.ok() )
		return type_decl_result;
	auto type = type_decl_result.unwrap();

	ir::Name alloca_name = ir::Name(var_name.part(0), type, ir::Name::NameKind::Var);
	ir::NameRef name = sema.names().add_name(alloca_name);

	auto ir_alloca = sema.builder().create_alloca(name.id(), type);

	if( let_node.rhs )
	{
		auto expr_result = sema_expr(sema, let_node.rhs);
		if( !expr_result.ok() )
			return expr_result;

		sema.builder().create_store(ir_alloca, expr_result.unwrap().rvalue().inst);
	}

	return ir::ActionResult();
}