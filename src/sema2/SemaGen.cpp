

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
	{
		auto ex = sema_fn(sema, ast);
		if( !ex.ok() )
			return ex;

		return sema.TLS(ex.unwrap());
	}
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

SemaResult<ir::IRStmt*>
sema::sema_stmt(Sema2& sema, ast::AstNode* ast)
{
	auto stmtr = expected(ast, ast::as_stmt);
	if( !stmtr.ok() )
		return stmtr;
	auto stmt = stmtr.unwrap();

	auto stmt_node = stmt.stmt;

	switch( stmt_node->type )
	{
	case NodeType::Return:
	{
		auto fn_callr = sema_return(sema, stmt_node);
		if( !fn_callr.ok() )
			return fn_callr;

		return sema.Stmt(fn_callr.unwrap());
	}
	case NodeType::Let:
	{
		auto letr = sema_let(sema, stmt_node);
		if( !letr.ok() )
			return letr;

		return sema.Stmt(letr.unwrap());
	}
	case NodeType::Assign:
	{
		auto fn_callr = sema_assign(sema, stmt_node);
		if( !fn_callr.ok() )
			return fn_callr;

		return sema.Stmt(fn_callr.unwrap());
	}
	default:
	{
		auto exprstmtr = sema_expr_any(sema, stmt_node);
		if( !exprstmtr.ok() )
			return exprstmtr;

		return sema.Stmt(exprstmtr.unwrap());
	}
	}

	return NotImpl();
}

SemaResult<ir::IRExpr*>
sema::sema_expr_any(Sema2& sema, ast::AstNode* expr_node)
{
	switch( expr_node->type )
	{
	case NodeType::FnCall:
	{
		auto fn_callr = sema_fn_call(sema, expr_node);
		if( !fn_callr.ok() )
			return fn_callr;

		return sema.Expr(fn_callr.unwrap());
	}
	case NodeType::NumberLiteral:
	{
		auto litr = sema_number_literal(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	case NodeType::StringLiteral:
	{
		auto litr = sema_string_literal(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	case NodeType::Id:
	{
		auto litr = sema_id(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	default:
		break;
	}

	return NotImpl();
}

SemaResult<ir::IRExpr*>
sema::sema_expr(Sema2& sema, ast::AstNode* ast)
{
	auto exprr = expected(ast, ast::as_expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	auto expr_node = expr.expr;

	return sema_expr_any(sema, expr_node);
}

static void
inject_function_args(sema::Sema2& sema, Vec<ir::IRValueDecl*>& args)
{
	for( auto arg : args )
		sema.add_value_identifier(*arg->name, arg->type_decl->type_instance);
}

SemaResult<ir::IRFunction*>
sema::sema_fn(Sema2& sema, ast::AstNode* ast)
{
	auto fnr = expected(ast, ast::as_fn);
	if( !fnr.ok() )
		return fnr;
	auto fn = fnr.unwrap();

	auto protor = sema_fn_proto(sema, fn.prototype);
	if( !protor.ok() )
		return protor;
	auto proto = protor.unwrap();

	auto maybe_return_type = proto->fn_type->get_return_type();
	assert(
		maybe_return_type.has_value() &&
		"Function prototype did not provide return type. (Missing infer?)");

	sema.push_scope();
	inject_function_args(sema, *proto->args);
	sema.set_expected_return(maybe_return_type.value());
	auto bodyr = sema_block(sema, fn.body, false);
	if( !bodyr.ok() )
		return bodyr;
	sema.clear_expected_return();
	sema.pop_scope();

	return sema.Fn(ast, proto, bodyr.unwrap());
}

SemaResult<ir::IRArgs*>
sema::sema_fn_args(Sema2& sema, ast::AstNode* ast)
{
	auto argsr = expected(ast, ast::as_expr_list);
	if( !argsr.ok() )
		return argsr;
	auto args = argsr.unwrap();

	auto argslist = sema.create_elist();
	for( auto argexpr : args.exprs )
	{
		auto exprr = sema_expr(sema, argexpr);
		if( !exprr.ok() )
			return exprr;

		argslist->push_back(exprr.unwrap());
	}

	return sema.Args(ast, argslist);
}

SemaResult<ir::IRCall*>
sema::sema_fn_call(Sema2& sema, ast::AstNode* ast)
{
	auto fn_callr = expected(ast, ast::as_fn_call);
	if( !fn_callr.ok() )
		return fn_callr;
	auto fn_call = fn_callr.unwrap();

	auto call_targetr = sema_expr(sema, fn_call.call_target);
	if( !call_targetr.ok() )
		return call_targetr;
	auto call_target = call_targetr.unwrap();

	auto argsr = sema_fn_args(sema, fn_call.args);
	if( !argsr.ok() )
		return argsr;

	return sema.FnCall(ast, call_target, argsr.unwrap());
}

SemaResult<ir::IRId*>
sema::sema_id(Sema2& sema, ast::AstNode* ast)
{
	auto idr = expected(ast, ast::as_id);
	if( !idr.ok() )
		return idr;
	auto id = idr.unwrap();

	auto maybe_type = sema.lookup_name(*id.name);
	if( !maybe_type.has_value() )
		return SemaError("Unrecognized variable '" + *id.name + "'");

	return sema.Id(ast, id.name, maybe_type.value());
}

SemaResult<ir::IRReturn*>
sema::sema_return(Sema2& sema, ast::AstNode* ast)
{
	auto returnr = expected(ast, ast::as_fn_return);
	if( !returnr.ok() )
		return returnr;
	auto return_expr = returnr.unwrap();

	auto retr = sema_expr(sema, return_expr.expr);
	if( !retr.ok() )
		return retr;
	auto ret = retr.unwrap();

	auto maybe_expected_type = sema.get_expected_return();
	if( !maybe_expected_type.has_value() )
		return SemaError("Return statement outside function?");

	auto expected_type = maybe_expected_type.value();
	if( !sema.types.equal_types(expected_type, ret->type_instance) )
		return SemaError("Incorrect return type.");

	return sema.Return(ast, retr.unwrap());
}

SemaResult<ir::IRLet*>
sema::sema_let(Sema2& sema, ast::AstNode* ast)
{
	//
	auto letr = expected(ast, ast::as_let);
	if( !letr.ok() )
		return letr;
	auto let = letr.unwrap();

	auto namer = as_name(sema, let.identifier);
	if( !namer.ok() )
		return namer;
	auto name = namer.unwrap();

	auto type_declrr = sema_type_decl(sema, let.type_declarator);
	if( !type_declrr.ok() )
		return type_declrr;
	auto type_declr = type_declrr.unwrap();

	auto rhs_exprr = sema_expr(sema, let.rhs);
	if( !rhs_exprr.ok() )
		return rhs_exprr;
	auto rhs = rhs_exprr.unwrap();

	if( !sema.types.equal_types(type_declr->type_instance, rhs->type_instance) )
		return SemaError(
			"Mismatched types: " + sema::to_string(type_declr->type_instance) +
			" != " + sema::to_string(rhs->type_instance));

	// TODO: Do I really need to do this?
	type_declr->type_instance =
		sema.types.non_inferred(type_declr->type_instance, rhs->type_instance);

	sema.add_value_identifier(*name, type_declr->type_instance);

	auto lhs_expr = sema.Expr(sema.ValueDecl(let.identifier, name, type_declr));
	return sema.Let(ast, name, sema.Assign(ast, ast::AssignOp::assign, lhs_expr, rhs));
}

SemaResult<ir::IRAssign*>
sema::sema_assign(Sema2& sema, ast::AstNode* ast)
{
	//
	auto assignr = expected(ast, ast::as_assign);
	if( !assignr.ok() )
		return assignr;
	auto assign = assignr.unwrap();

	auto lhs_exprr = sema_expr(sema, assign.left);
	if( !lhs_exprr.ok() )
		return lhs_exprr;
	auto lhs = lhs_exprr.unwrap();

	auto rhs_exprr = sema_expr(sema, assign.right);
	if( !rhs_exprr.ok() )
		return rhs_exprr;
	auto rhs = rhs_exprr.unwrap();

	if( !sema.types.equal_types(lhs->type_instance, rhs->type_instance) )
		return SemaError(
			"Mismatched types: " + sema::to_string(lhs->type_instance) +
			" != " + sema::to_string(rhs->type_instance));

	return sema.Assign(ast, assign.op, lhs, rhs);
}

SemaResult<ir::IRBlock*>
sema::sema_block(Sema2& sema, ast::AstNode* ast, bool new_scope)
{
	if( new_scope )
		sema.push_scope();

	auto blockr = expected(ast, ast::as_block);
	if( !blockr.ok() )
		return blockr;
	auto block = blockr.unwrap();

	auto stmtlist = sema.create_slist();
	for( auto stmt : block.statements )
	{
		auto stmtr = sema_stmt(sema, stmt);
		if( !stmtr.ok() )
			return stmtr;

		stmtlist->push_back(stmtr.unwrap());
	}

	if( new_scope )
		sema.pop_scope();

	return sema.Block(ast, stmtlist);
}

SemaResult<ir::IRExternFn*>
sema::sema_extern_fn(Sema2& sema, ast::AstNode* ast)
{
	auto extern_fnr = expected(ast, ast::as_extern_fn);
	if( !extern_fnr.ok() )
		return extern_fnr;
	auto extern_fn = extern_fnr.unwrap();

	auto protor = sema_fn_proto(sema, extern_fn.prototype);
	if( !protor.ok() )
		return protor;

	return sema.ExternFn(ast, protor.unwrap());
}

static Vec<TypedMember>
params_to_members(Vec<ir::IRValueDecl*>* params)
{
	Vec<TypedMember> mems;

	for( auto param : *params )
	{
		mems.emplace_back(param->type_decl->type_instance, *param->name);
	}
	return mems;
}

SemaResult<ir::IRProto*>
sema::sema_fn_proto(Sema2& sema, ast::AstNode* ast)
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

	auto rt = rt_type_declr.unwrap();

	auto fn_type =
		sema.CreateType(Type::Function(*name, params_to_members(argslist), rt->type_instance));
	sema.add_type_identifier(fn_type);
	sema.add_value_identifier(*name, TypeInstance::OfType(fn_type));

	return sema.Proto(ast, name, argslist, rt, fn_type);
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

SemaResult<ir::IRNumberLiteral*>
sema::sema_number_literal(Sema2& sema, ast::AstNode* ast)
{
	auto numr = expected(ast, ast::as_number_literal);
	if( !numr.ok() )
		return numr;
	auto num = numr.unwrap();

	return sema.NumberLiteral(ast, TypeInstance::OfType(sema.types.i32_type()), num.literal);
}

SemaResult<ir::IRStringLiteral*>
sema::sema_string_literal(Sema2& sema, ast::AstNode* ast)
{
	auto numr = expected(ast, ast::as_string_literal);
	if( !numr.ok() )
		return numr;
	auto num = numr.unwrap();

	auto name = sema.create_name(num.literal->c_str(), num.literal->size());

	return sema.StringLiteral(ast, TypeInstance::PointerTo(sema.types.i8_type(), 1), name);
}

SemaResult<ir::IRTypeDeclaraor*>
sema::sema_type_decl(Sema2& sema, ast::AstNode* ast)
{
	auto type_declr = expected(ast, ast::as_type_decl);
	if( !type_declr.ok() )
		return type_declr;
	auto type_decl = type_declr.unwrap();

	if( !type_decl.empty )
	{
		auto type = sema.lookup_type(*type_decl.name);
		if( !type )
			return SemaError("Could not find type '" + *type_decl.name + "'");

		auto type_instance = TypeInstance::PointerTo(type, type_decl.indirection_level);

		return sema.TypeDecl(ast, type_instance);
	}
	else
	{
		return sema.TypeDecl(ast, TypeInstance::OfType(sema.types.infer_type()));
	}
}