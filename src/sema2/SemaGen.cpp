

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
	case NodeType::Struct:
	{
		auto ex = sema_struct(sema, ast);
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
	case NodeType::If:
	{
		auto ifr = sema_if(sema, stmt_node);
		if( !ifr.ok() )
			return ifr;

		return sema.Stmt(ifr.unwrap());
	}
	case NodeType::Else:
	{
		auto ifr = sema_else(sema, stmt_node);
		if( !ifr.ok() )
			return ifr;

		return sema.Stmt(ifr.unwrap());
	}
	case NodeType::Block:
	{
		auto ifr = sema_block(sema, stmt_node, true);
		if( !ifr.ok() )
			return ifr;

		return sema.Stmt(ifr.unwrap());
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

SemaResult<ir::IRIf*>
sema::sema_if(Sema2& sema, ast::AstNode* ast)
{
	auto result = expected(ast, ast::as_if);
	if( !result.ok() )
		return result;

	auto ifcond = result.unwrap();

	auto exprr = sema_expr(sema, ifcond.condition);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	if( !sema.types.equal_types(expr->type_instance, sema.types.BoolType()) )
		return SemaError("If condition expression must be bool type.");

	auto stmtr = sema_stmt(sema, ifcond.then_block);
	if( !stmtr.ok() )
		return stmtr;
	auto stmt = stmtr.unwrap();

	if( ifcond.else_block )
	{
		auto else_stmtr = sema_else(sema, ifcond.else_block);
		if( !else_stmtr.ok() )
			return else_stmtr;

		return sema.If(ast, expr, stmt, else_stmtr.unwrap());
	}
	else
	{
		return sema.If(ast, expr, stmt, nullptr);
	}
}

SemaResult<ir::IRElse*>
sema::sema_else(Sema2& sema, ast::AstNode* ast)
{
	auto result = expected(ast, ast::as_else);
	if( !result.ok() )
		return result;

	auto else_stmt = result.unwrap();

	auto stmtr = sema_stmt(sema, else_stmt.stmt);
	if( !stmtr.ok() )
		return stmtr;

	return sema.Else(ast, stmtr.unwrap());
}

SemaResult<ir::IRValueDecl*>
sema::sema_struct_tls(Sema2& sema, ast::AstNode* ast)
{
	// TODO: variadic type
	return sema_value_decl(sema, ast);
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
	case NodeType::Expr:
	{
		auto litr = sema_expr(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return litr.unwrap();
	}
	case NodeType::BinOp:
	{
		auto litr = sema_binop(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	case NodeType::MemberAccess:
	{
		auto litr = sema_member_access(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	case NodeType::AddressOf:
	{
		auto litr = sema_addressof(sema, expr_node);
		if( !litr.ok() )
			return litr;

		return sema.Expr(litr.unwrap());
	}
	case NodeType::Deref:
	{
		auto litr = sema_deref(sema, expr_node);
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
inject_function_args(sema::Sema2& sema, Vec<ir::IRParam*>& args)
{
	for( auto arg : args )
	{
		if( arg->type == ir::IRParamType::ValueDecl )
		{
			auto value_decl = arg->data.value_decl;
			sema.add_value_identifier(*value_decl->name, value_decl->type_decl->type_instance);
		}
	}
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

	if( !call_target->type_instance.type->is_function_type() )
		return SemaError("...is not a function!");

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

	auto maybe_value = sema.lookup_name(*id.name);
	if( maybe_value.has_value() )
		return sema.Id(ast, id.name, maybe_value.value());

	// Struct name?
	auto maybe_struct = sema.lookup_type(*id.name);
	if( maybe_struct && maybe_struct->is_struct_type() )
	{
		// Lookup constructor.
		// Note that we need to generate a constructor?
		auto str_name = maybe_struct->get_name();
		auto name = sema.create_name(str_name.c_str(), str_name.size());
		return sema.Id(ast, name, TypeInstance::OfType(maybe_struct));
	}

	return SemaError("Unrecognized variable '" + *id.name + "'");
}

SemaResult<ir::IRMemberAccess*>
sema::sema_member_access(Sema2& sema, ast::AstNode* ast)
{
	auto mar = expected(ast, ast::as_member_access);
	if( !mar.ok() )
		return mar;
	auto ma = mar.unwrap();

	auto namer = as_name(sema, ma.member_name);
	if( !namer.ok() )
		return namer;
	auto name = namer.unwrap();

	auto val_exprr = sema_expr(sema, ma.expr);
	if( !val_exprr.ok() )
		return val_exprr;
	auto val_expr = val_exprr.unwrap();

	auto expr_type = val_expr->type_instance;
	if( !expr_type.type->is_struct_type() || expr_type.indirection_level != 0 )
		return SemaError("Cannot access member '" + *name + "' of '" + to_string(expr_type) + "'");

	auto member = expr_type.type->get_member(*name);
	if( !member.has_value() )
		return SemaError(
			"Cannot access member '" + *name + "' of '" + to_string(expr_type) + "' because '" +
			*name + "' does not exist");
	auto member_type = member.value();
	return sema.MemberAccess(ast, val_expr, member_type.type, name);
}

SemaResult<ir::IRAddressOf*>
sema::sema_addressof(Sema2& sema, ast::AstNode* ast)
{
	auto addrofr = expected(ast, ast::as_address_of);
	if( !addrofr.ok() )
		return addrofr;
	auto addrof = addrofr.unwrap();

	auto exprr = sema_expr(sema, addrof.expr);
	if( !exprr.ok() )
		return exprr;

	auto expr = exprr.unwrap();

	return sema.AddressOf(ast, expr, expr->type_instance.PointerTo(1));
}

SemaResult<ir::IRDeref*>
sema::sema_deref(Sema2& sema, ast::AstNode* ast)
{
	auto derefr = expected(ast, ast::as_deref);
	if( !derefr.ok() )
		return derefr;
	auto deref = derefr.unwrap();

	auto exprr = sema_expr(sema, deref.expr);
	if( !exprr.ok() )
		return exprr;

	auto expr = exprr.unwrap();

	return sema.Deref(ast, expr, expr->type_instance.PointerElementType());
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

static TypeInstance
binop_type(Sema2& sema, ast::BinOp op, ir::IRExpr* lhs, ir::IRExpr* rhs)
{
	switch( op )
	{
	case ast::BinOp::plus:
	case ast::BinOp::star:
	case ast::BinOp::minus:
	case ast::BinOp::slash:
		return lhs->type_instance;
	case ast::BinOp::gt:
	case ast::BinOp::gte:
	case ast::BinOp::lt:
	case ast::BinOp::lte:
	case ast::BinOp::and_op:
	case ast::BinOp::or_op:
	case ast::BinOp::cmp:
	case ast::BinOp::ne:
		return sema.types.BoolType();
	case ast::BinOp::bad:
		assert(0);
		break;
	}
}

SemaResult<ir::IRBinOp*>
sema::sema_binop(Sema2& sema, ast::AstNode* ast)
{
	auto binopr = expected(ast, ast::as_binop);
	if( !binopr.ok() )
		return binopr;
	auto binop = binopr.unwrap();

	auto lhs_exprr = sema_expr(sema, binop.left);
	if( !lhs_exprr.ok() )
		return lhs_exprr;
	auto lhs = lhs_exprr.unwrap();

	auto rhs_exprr = sema_expr(sema, binop.right);
	if( !rhs_exprr.ok() )
		return rhs_exprr;
	auto rhs = rhs_exprr.unwrap();

	// TODO: Int conversions?
	if( !sema.types.equal_types(lhs->type_instance, rhs->type_instance) )
		return SemaError(
			"Mismatched types: " + sema::to_string(lhs->type_instance) +
			" != " + sema::to_string(rhs->type_instance));

	auto type = binop_type(sema, binop.op, lhs, rhs);

	return sema.BinOp(ast, binop.op, lhs, rhs, type);
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

struct params_to_members_t
{
	std::vector<TypedMember> vec;
	bool is_var_arg;
};

static sema::SemaResult<params_to_members_t>
params_to_members(Vec<ir::IRParam*>* params)
{
	params_to_members_t result;

	int idx = 0;
	for( auto param : *params )
	{
		switch( param->type )
		{
		case ir::IRParamType::ValueDecl:
		{
			auto value_decl = param->data.value_decl;
			// TODO: Should functions have named members too?
			result.vec.emplace_back(value_decl->type_decl->type_instance, *value_decl->name, idx++);
			break;
		}
		case ir::IRParamType::VarArg:
			result.is_var_arg = true;
			idx++;
			goto done;
		}
	}

done:
	if( idx != params->size() )
		return SemaError("Cannot have varargs before named args.");

	return result;
}

static std::map<String, TypedMember>
members_to_members(std::map<String, ir::IRValueDecl*>& params)
{
	std::map<String, TypedMember> map;

	int idx = 0;
	for( auto param : params )
	{
		// TODO: Should functions have named members too?
		map.emplace(
			param.first, TypedMember(param.second->type_decl->type_instance, param.first, idx++));
	}
	return map;
}

SemaResult<ir::IRParam*>
sema::sema_fn_param(Sema2& sema, ast::AstNode* ast)
{
	switch( ast->type )
	{
	case AstValueDecl::nt:
	{
		auto value_declr = sema_value_decl(sema, ast);
		if( !value_declr.ok() )
			return value_declr;

		return sema.IRParam(ast, value_declr.unwrap());
	}
	case AstVarArg::nt:
		return sema.IRParam(ast, sema.VarArg(ast));
	}

	return NotImpl();
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
		auto paramr = sema_fn_param(sema, arg);
		if( !paramr.ok() )
			return paramr;

		argslist->push_back(paramr.unwrap());
	}

	auto rt_type_declr = sema_type_decl(sema, fn_proto.return_type);
	if( !rt_type_declr.ok() )
		return rt_type_declr;

	auto rt = rt_type_declr.unwrap();

	auto membersr = params_to_members(argslist);
	if( !membersr.ok() )
		return membersr;
	auto members = membersr.unwrap();
	auto fn_type = sema.CreateType(Type::Function(*name, members.vec, rt->type_instance));
	sema.add_type_identifier(fn_type);
	sema.add_value_identifier(*name, TypeInstance::OfType(fn_type));

	return sema.Proto(ast, name, argslist, rt, fn_type);
}

SemaResult<ir::IRStruct*>
sema::sema_struct(Sema2& sema, ast::AstNode* ast)
{
	auto structr = expected(ast, ast::as_struct);
	if( !structr.ok() )
		return structr;
	auto struct_node = structr.unwrap();

	auto namer = as_name(sema, struct_node.type_name);
	if( !namer.ok() )
		return namer;

	auto name = namer.unwrap();

	auto members = sema.create_member_map();
	for( auto stmt : struct_node.members )
	{
		auto memberr = sema_struct_tls(sema, stmt);
		if( !memberr.ok() )
			return memberr;

		auto member = memberr.unwrap();

		members->emplace(*member->name, member);
	}

	auto fn_type = sema.CreateType(Type::Struct(*name, members_to_members(*members)));
	sema.add_type_identifier(fn_type);

	return sema.Struct(ast, fn_type, members);
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

SemaResult<ir::IRFunction*>
sema::generate_constructor(Sema2& sema, ast::AstNode* ast)
{
	return NotImpl();
}