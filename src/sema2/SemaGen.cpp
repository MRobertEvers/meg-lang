

#include "SemaGen.h"

#include "ast2/AstCasts.h"

using namespace sema;
using namespace ir;
using namespace ast;

static SemaError
NotImpl()
{
	return SemaError("Not Implemented.");
}

static SemaResult<String*>
as_name(Sema2& sema, ast::AstNode* ast)
{
	auto idr = expected(ast, ast::as_id);
	if( !idr.ok() )
		return idr;
	auto id = idr.unwrap();

	return sema.create_name(id.name->c_str(), id.name->size());
}

// struct sema_extern_fn_proto_t
// {
// 	Type const* fn_type;
// 	Vec<TypedMember> values;
// };

// static SemaResult<sema_extern_fn_proto_t>
// sema_extern_fn_proto(Sema2& sema, ast::AstNode* node)
// {
// 	auto fn_protor = expected(node, ast::as_fn_proto);
// 	if( !fn_protor.ok() )
// 		return fn_protor;

// 	auto idnode = as_id(sema, mod.name);
// 	if( !idnode.ok() )
// 		return result;

// 	auto fn_name = idnode.unwrap()->name;

// 	auto params = sema.sema_fn_param_list(mod.params);
// 	if( !params.ok() )
// 		return params;

// 	auto retnode = typecheck_type_decl(sema, mod.return_type);
// 	if( !retnode.ok() )
// 		return retnode;

// 	auto return_type = retnode.unwrap();

// 	sema.current_scope->set_expected_return(return_type);

// 	auto newtype = Type::Function(*fn_name, params.unwrap(), return_type);
// 	auto fn_type = sema.CreateType(newtype);
// 	sema.add_type_identifier(fn_type);
// 	sema.add_value_identifier(*fn_name, TypeInstance::OfType(fn_type));

// 	return (struct sema_extern_fn_proto_t){
// 		.fn_type = fn_type,
// 		.values = params.unwrap() //
// 	};
// }

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
	switch( ast->type )
	{
	case NodeType::Fn:
		return NotImpl();
	case NodeType::ExternFn:
	{
		auto ex = sema_extern_fn(sema, ast);
		if( !ex.ok() )
			return ex;

		return sema.TLS(ex.unwrap());
	}
	default:
		return SemaError("Unsupported NodeType as TLS.");
	}
}

SemaResult<ir::IRExternFn*>
sema::sema_extern_fn(Sema2& sema, ast::AstNode* ast)
{
	auto extern_fnr = expected(ast, ast::as_extern_fn);
	if( !extern_fnr.ok() )
		return extern_fnr;

	return NotImpl();
}

SemaResult<ir::IRProto*>
sema_fn_proto(Sema2& sema, ast::AstNode* ast)
{
	auto fn_protor = expected(ast, ast::as_fn_proto);
	if( !fn_protor.ok() )
		return fn_protor;
	auto fn_proto = fn_protor.unwrap();

	auto idr = expected(fn_proto.name, ast::as_id);
	if( !idr.ok() )
		return idr;
	auto id = idr.unwrap();

	auto name = sema.create_name(id.name->c_str(), id.name->size());

	auto argsr = expected(fn_proto.params, ast::as_fn_param_list);
	if( !argsr.ok() )
		return argsr;
	auto args = argsr.unwrap();

	auto argslist = sema.create_argslist();
	for( auto arg : args.params )
	{
		auto value_declr = sema_value_decl(sema, arg);
		if( !value_declr.ok() )
			return value_declr;

		argslist->push_back(value_declr.unwrap());
	}

	auto rt_type_declr = sema_type_decl(sema, fn_proto.return_type);
	if( !rt_type_declr.ok() )
		return rt_type_declr;

	// TODO: Emit function type and proto
	auto rt = rt_type_declr.unwrap();

	return sema.Proto(ast, name, argslist, rt);
}

SemaResult<ir::IRValueDecl*>
sema::sema_value_decl(Sema2& sema, ast::AstNode* ast)
{
	auto value_declr = expected(ast, ast::as_value_decl);
	if( !value_declr.ok() )
		return value_declr;
	auto value_decl = value_declr.unwrap();

	auto namer = as_name(sema, value_decl.name);
	if( !namer.ok() )
		return namer;
	auto name = namer.unwrap();

	auto type_declr = sema_type_decl(sema, value_decl.type_name);
	if( !type_declr.ok() )
		return type_declr;

	return sema.ValueDecl(ast, name, type_declr.unwrap());
}

SemaResult<ir::IRTypeDeclaraor*>
sema::sema_type_decl(Sema2& sema, ast::AstNode* ast)
{
	auto type_declr = expected(ast, ast::as_type_decl);
	if( !type_declr.ok() )
		return type_declr;
	auto type_decl = type_declr.unwrap();

	auto type = sema.lookup_type(*type_decl.name);
	if( !type )
		return SemaError("Could not find type '" + *type_decl.name + "'");

	auto type_instance = TypeInstance::PointerTo(type, type_decl.indirection_level);

	return sema.TypeDecl(ast, type_instance);
}