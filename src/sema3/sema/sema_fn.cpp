

#include "sema_fn.h"

#include "../SemaLookup.h"
#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "idname.h"
#include "sema_type_decl.h"

using namespace sema;

struct sema_fn_params_t
{
	std::vector<MemberTypeInstance> arg_types;
	bool is_var_arg;
};

/**
 * @brief This is not a public function because functions are created atomically.
 *
 * @param sema
 * @param ast
 * @return SemaResult<sema_fn_param_t>
 */
static SemaResult<sema_fn_params_t>
sema_fn_params(Sema& sema, ast::AstNode* ast)
{
	auto ast_fn_params = expected(ast, ast::as_fn_param_list);
	if( !ast_fn_params.ok() )
		return ast_fn_params;
	auto fn_params_node = ast_fn_params.unwrap();

	sema_fn_params_t result;
	int idx = 0;
	for( auto param_ast : fn_params_node.params )
	{
		switch( param_ast->type )
		{
		case ast::AstValueDecl::nt:
		{
			auto ast_value_decl = expected(ast, ast::as_value_decl);
			if( !ast_value_decl.ok() )
				return ast_value_decl;
			auto value_decl_node = ast_value_decl.unwrap();

			auto type_decl_result = sema_type_decl(sema, value_decl_node.type_name);
			if( !type_decl_result.ok() )
				return type_decl_result;

			auto ast_id = expected(ast, ast::as_id);
			if( !ast_id.ok() )
				return ast_id;
			auto id_node = ast_id.unwrap();

			auto param_name = idname(id_node);
			// TODO: Assert simple name

			result.arg_types.push_back(
				MemberTypeInstance(type_decl_result.unwrap(), param_name.part(0), idx));

			break;
		}
		case ast::AstVarArg::nt:
			result.is_var_arg = true;
			break;
		}

		idx += 1;
	}

	return result;
}

SemaResult<ir::ActionResult>
sema::sema_fn_proto(Sema& sema, ast::AstNode* ast)
{
	auto ast_fn_proto = expected(ast, ast::as_fn_proto);
	if( !ast_fn_proto.ok() )
		return ast_fn_proto;
	auto fn_proto_node = ast_fn_proto.unwrap();

	auto ast_id = expected(fn_proto_node.name, ast::as_id);
	if( !ast_id.ok() )
		return ast_id;
	auto id_node = ast_id.unwrap();

	// TODO: Assert simple name?
	auto name = idname(id_node);

	NameLookupResult lu_result = sema.names().lookup(idname(id_node));
	if( lu_result.is_found() )
		return SemaError("Redefinition of " + name.to_string());

	auto params_result = sema_fn_params(sema, fn_proto_node.params);
	if( !params_result.ok() )
		return params_result;
	auto params = params_result.unwrap();

	auto return_type_result = sema_type_decl(sema, fn_proto_node.return_type);
	if( !return_type_result.ok() )
		return return_type_result;
	auto return_type = return_type_result.unwrap();

	// TODO: Confusing that the members are part of the type and the namespace.
	// The type should really just contain the types and not the names.
	auto fn_type = Type::Function(name.part(0), params.arg_types, return_type, params.is_var_arg);
	auto ir_fn_type = sema.create_type(fn_type);

	// This is how name resolution is done.
	for( auto param : params.arg_types )
		ir_fn_type.add_name(Name(param.name, param.type, Name::Member));

	sema.emit(new ir::FnDecl(ir_fn_type, ir::FnDecl::Extern));

	return ir::ActionResult();
}

SemaResult<ir::ActionResult>
sema::sema_fn(Sema& sema, ast::AstNode* ast)
{
	//
	// auto ast_fn = expected(ast, ast::as_fn);
	// if( !ast_fn.ok() )
	// 	return ast_fn;
	// auto fn_node = ast_fn.unwrap();

	// auto ast_proto = sema_fn_proto(sema, fn_node.prototype);
	// if( !ast_proto.ok() )
	// 	return ast_proto;
	// auto proto_node = ast_proto.unwrap();
}

SemaResult<ir::ActionResult>
sema::sema_extern_fn(Sema& sema, ast::AstNode* ast)
{
	return sema_fn(sema, ast);
}