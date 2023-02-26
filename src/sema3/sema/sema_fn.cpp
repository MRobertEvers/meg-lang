

#include "sema_fn.h"

#include "../sema_expected.h"
#include "ast2/AstCasts.h"
#include "idname.h"
#include "sema_block.h"
#include "sema_type_decl.h"

using namespace sema;
using namespace ir;

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
			auto ast_value_decl = expected(param_ast, ast::as_value_decl);
			if( !ast_value_decl.ok() )
				return ast_value_decl;
			auto value_decl_node = ast_value_decl.unwrap();

			auto type_decl_result = sema_type_decl(sema, value_decl_node.type_name);
			if( !type_decl_result.ok() )
				return type_decl_result;

			auto ast_id = expected(value_decl_node.name, ast::as_id);
			if( !ast_id.ok() )
				return ast_id;
			auto id_node = ast_id.unwrap();

			// TODO: Assert simple name
			auto param_name = idname(id_node);

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

static SemaResult<ir::NameRef>
sema_fn_decl(Sema& sema, ast::AstNode* ast)
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

	NameLookupResult lu_result = sema.names().lookup(name);
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
	NameRef ir_fn_type = sema.create_type(fn_type);

	// TODO: Confusing, names are defined during FnDef.
	// This is how name resolution is done.
	// for( auto param : params.arg_types )
	// 	ir_fn_type.add_name(Name(param.name, param.type, Name::Member));

	return ir_fn_type;
}

SemaResult<ir::FnDecl*>
sema::sema_fn_proto(Sema& sema, ast::AstNode* ast)
{
	auto fn_decl = sema_fn_decl(sema, ast);
	if( !fn_decl.ok() )
		return fn_decl;

	auto fn_name = fn_decl.unwrap();
	auto fn_type = fn_name.type();

	return sema.builder().create_fn_decl(fn_name.id(), fn_name.type());
}

SemaResult<ir::ActionResult>
sema::sema_fn(Sema& sema, ast::AstNode* ast)
{
	auto ast_fn = expected(ast, ast::as_fn);
	if( !ast_fn.ok() )
		return ast_fn;
	auto fn_node = ast_fn.unwrap();

	auto fn_proto_result = sema_fn_proto(sema, fn_node.prototype);
	if( !fn_proto_result.ok() )
		return fn_proto_result;
	auto fn_proto = fn_proto_result.unwrap();
	auto fn_type = fn_proto->type;

	// We add the arg names here in case of separate decl and def.
	// The argument names need not match.
	std::vector<NameId> arg_names;
	auto scope = sema.names().get(fn_proto->name_id);
	for( int i = 0; i < fn_type.type->get_member_count(); i++ )
	{
		auto member = fn_type.type->get_member(i);
		auto name = sema.names().add_name(
			scope, Name(member.name, scope.id(), member.type, ir::Name::NameKind::Var));
		arg_names.push_back(name.id());
	}

	ir::FnDef* ir_fn = sema.builder().create_fn(arg_names, fn_type);
	ir::BasicBlock* entry = sema.builder().create_basic_block(ir_fn);
	sema.builder().set_insert_point(entry);

	sema.names().push_scope(scope);

	auto fn_body_result = sema_block(sema, fn_node.body);
	if( !fn_body_result.ok() )
		return fn_body_result;

	sema.names().pop_scope();

	return ir::ActionResult();
}

SemaResult<ir::ActionResult>
sema::sema_extern_fn(Sema& sema, ast::AstNode* ast)
{
	auto ast_extern_fn = expected(ast, ast::as_extern_fn);
	if( !ast_extern_fn.ok() )
		return ast_extern_fn;
	sema_fn_proto(sema, ast_extern_fn.unwrap().prototype);

	return ir::ActionResult();
}