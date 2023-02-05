
#include "Sema2.h"

#include <iostream>

using namespace ast;
using namespace sema;

Sema2::Sema2()
{
	scopes.reserve(100);
	scopes.emplace_back(&types);
	current_scope = &scopes.back();

	for( auto& ty : types.types )
	{
		auto second = &ty.second;
		add_type_identifier(second);
	}
}

Scope*
Sema2::push_scope()
{
	auto s = Scope(current_scope);
	scopes.push_back(s);
	current_scope = &scopes.back();

	return current_scope;
}

void
Sema2::pop_scope()
{
	current_scope->is_in_scope = false;
	current_scope = current_scope->get_parent();
}

Type const*
Sema2::CreateType(Type ty)
{
	return types.define_type(ty);
}

void
Sema2::add_value_identifier(String const& name, TypeInstance id)
{
	return current_scope->add_value_identifier(name, id);
}

void
Sema2::add_type_identifier(Type const* id)
{
	return current_scope->add_type_identifier(id);
}

std::optional<TypeInstance>
Sema2::get_expected_return()
{
	return current_scope->get_expected_return();
}

void
Sema2::set_expected_return(TypeInstance n)
{
	current_scope->set_expected_return(n);
}

void
Sema2::clear_expected_return()
{
	current_scope->clear_expected_return();
}

std::optional<TypeInstance>
Sema2::lookup_name(String const& name)
{
	auto ti = current_scope->lookup_value_type(name);
	if( ti == nullptr )
		return std::optional<TypeInstance>();

	return *ti;
}

Type const*
Sema2::lookup_type(String const& name)
{
	return current_scope->lookup_type(name);
}

Vec<ir::IRTopLevelStmt*>*
Sema2::create_tlslist()
{
	//
	return new Vec<ir::IRTopLevelStmt*>();
}

Vec<ir::IRStmt*>*
Sema2::create_slist()
{
	//
	return new Vec<ir::IRStmt*>();
}

Vec<ir::IRExpr*>*
Sema2::create_elist()
{
	return new Vec<ir::IRExpr*>();
}

Vec<ir::IRValueDecl*>*
Sema2::create_argslist()
{
	return new Vec<ir::IRValueDecl*>();
}

String*
Sema2::create_name(char const* s, int size)
{
	return new String(s, size);
}

ir::IRModule*
Sema2::Module(AstNode* node, Vec<ir::IRTopLevelStmt*>* stmts)
{
	//
	auto mod = new ir::IRModule;

	mod->node = node;
	mod->stmts = stmts;

	return mod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRExternFn* fn)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = fn->node;
	nod->stmt.extern_fn = fn;
	nod->type = ir::IRTopLevelType::ExternFn;

	return nod;
}

ir::IRTopLevelStmt*
Sema2::TLS(ir::IRFunction* fn)
{
	auto nod = new ir::IRTopLevelStmt;

	nod->node = fn->node;
	nod->stmt.fn = fn;
	nod->type = ir::IRTopLevelType::Function;

	return nod;
}

ir::IRFunction*
Sema2::Fn(ast::AstNode* node, ir::IRProto* proto, ir::IRBlock* block)
{
	auto nod = new ir::IRFunction;

	nod->node = node;
	nod->proto = proto;
	nod->block = block;

	return nod;
}

ir::IRCall*
Sema2::FnCall(ast::AstNode* node, ir::IRExpr* call_target, ir::IRArgs* args)
{
	auto nod = new ir::IRCall;

	nod->node = node;
	nod->call_target = call_target;
	nod->args = args;

	return nod;
}

ir::IRExternFn*
Sema2::ExternFn(ast::AstNode* node, ir::IRProto* proto)
{
	auto nod = new ir::IRExternFn;

	nod->node = node;
	nod->proto = proto;

	return nod;
}

ir::IRProto*
Sema2::Proto(
	ast::AstNode* node,
	String* name,
	Vec<ir::IRValueDecl*>* args,
	ir::IRTypeDeclaraor* rt,
	Type const* fn_type)
{
	auto nod = new ir::IRProto;

	nod->node = node;
	nod->name = name;
	nod->args = args;
	nod->rt = rt;
	nod->fn_type = fn_type;

	return nod;
}

ir::IRBlock*
Sema2::Block(ast::AstNode* node, Vec<ir::IRStmt*>* stmts)
{
	auto nod = new ir::IRBlock;

	nod->node = node;
	nod->stmts = stmts;

	return nod;
}

ir::IRReturn*
Sema2::Return(ast::AstNode* node, ir::IRExpr* expr)
{
	auto nod = new ir::IRReturn;

	nod->node = node;
	nod->expr = expr;

	return nod;
}

ir::IRValueDecl*
Sema2::ValueDecl(ast::AstNode* node, String* name, ir::IRTypeDeclaraor* rt)
{
	auto nod = new ir::IRValueDecl;

	nod->node = node;
	nod->name = name;
	nod->type_decl = rt;

	return nod;
}

ir::IRTypeDeclaraor*
Sema2::TypeDecl(ast::AstNode* node, sema::TypeInstance type)
{
	auto nod = new ir::IRTypeDeclaraor;

	nod->node = node;
	nod->type_instance = type;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRCall* call)
{
	auto nod = new ir::IRExpr;

	nod->node = call->node;
	nod->expr.call = call;
	nod->type = ir::IRExprType::Call;

	auto t = call->call_target->type_instance.type->get_return_type();
	assert(t.has_value());
	nod->type_instance = t.value();

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRNumberLiteral* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.num_literal = nl;
	nod->type = ir::IRExprType::NumberLiteral;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRStringLiteral* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.str_literal = nl;
	nod->type = ir::IRExprType::StringLiteral;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRId* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.id = nl;
	nod->type = ir::IRExprType::Id;
	nod->type_instance = nl->type_instance;

	return nod;
}

ir::IRExpr*
Sema2::Expr(ir::IRValueDecl* nl)
{
	auto nod = new ir::IRExpr;

	nod->node = nl->node;
	nod->expr.decl = nl;
	nod->type = ir::IRExprType::ValueDecl;
	nod->type_instance = nl->type_decl->type_instance;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRReturn* ret)
{
	auto nod = new ir::IRStmt;

	nod->node = ret->node;
	nod->stmt.ret = ret;
	nod->type = ir::IRStmtType::Return;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRExpr* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.expr = expr;
	nod->type = ir::IRStmtType::ExprStmt;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRLet* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.let = expr;
	nod->type = ir::IRStmtType::Let;

	return nod;
}

ir::IRStmt*
Sema2::Stmt(ir::IRAssign* expr)
{
	auto nod = new ir::IRStmt;

	nod->node = expr->node;
	nod->stmt.assign = expr;
	nod->type = ir::IRStmtType::Assign;

	return nod;
}

ir::IRArgs*
Sema2::Args(ast::AstNode* node, Vec<ir::IRExpr*>* args)
{
	auto nod = new ir::IRArgs;

	nod->node = node;
	nod->args = args;

	return nod;
}

ir::IRNumberLiteral*
Sema2::NumberLiteral(ast::AstNode* node, sema::TypeInstance type_instance, long long val)
{
	auto nod = new ir::IRNumberLiteral;

	nod->node = node;
	nod->val = val;
	nod->type_instance = type_instance;

	return nod;
}

ir::IRStringLiteral*
Sema2::StringLiteral(ast::AstNode* node, sema::TypeInstance type_instance, String* name)
{
	auto nod = new ir::IRStringLiteral;

	nod->node = node;
	nod->value = name;
	nod->type_instance = type_instance;

	return nod;
}

ir::IRId*
Sema2::Id(ast::AstNode* node, String* name, sema::TypeInstance type)
{
	auto nod = new ir::IRId;

	nod->node = node;
	nod->name = name;
	nod->type_instance = type;

	return nod;
}

ir::IRLet*
Sema2::Let(ast::AstNode* node, String* name, ir::IRAssign* assign)
{
	auto nod = new ir::IRLet;

	nod->node = node;
	nod->name = name;
	nod->assign = assign;

	return nod;
}

ir::IRAssign*
Sema2::Assign(ast::AstNode* node, ast::AssignOp op, ir::IRExpr* lhs, ir::IRExpr* rhs)
{
	auto nod = new ir::IRAssign;

	nod->op = op;
	nod->node = node;
	nod->lhs = lhs;
	nod->rhs = rhs;

	return nod;
}

// static SemaResult<TypeInstance>
// expected(Sema2& sema, ast::AstNode* node, ast::NodeType type)
// {
// 	if( node->type != type )
// 		return SemaError("Expected type '" + to_string(type) + "'");

// 	auto semtag = sema.query(node);
// 	semtag->scope = sema.current_scope;
// 	semtag->tag_type = SemaTagType::Sema;

// 	return TypeInstance::OfType(sema.types.void_type());
// }

// static SemaResult<AstId*>
// as_id(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstId::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.id;
// }

// static SemaResult<AstBlock*>
// as_block(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstBlock::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.block;
// }

// static SemaResult<AstTypeDeclarator*>
// as_type_decl(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstTypeDeclarator::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.type_declarator;
// }

// static SemaResult<AstValueDecl*>
// as_value_decl(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstValueDecl::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.value_decl;
// }

// static SemaResult<AstFnCall*>
// as_fn_call(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstFnCall::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.fn_call;
// }

// static SemaResult<AstStmt*>
// as_stmt(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstStmt::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.stmt;
// }

// static SemaResult<AstExpr*>
// as_expr(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, AstExpr::nt);
// 	if( !result.ok() )
// 		return result;

// 	return &node->data.expr;
// }

// static SemaResult<TypeInstance>
// typecheck_type_decl(Sema2& sema, ast::AstNode* node)
// {
// 	auto type_name_result = as_type_decl(sema, node);
// 	if( !type_name_result.ok() )
// 		return type_name_result;

// 	// static_assert(type_name_result.unwrap()->classification == IdClassification::TypeIdentifier);
// 	auto declarator = type_name_result.unwrap();
// 	auto type_name = declarator->name;

// 	auto type = sema.current_scope->lookup_type(*type_name);
// 	if( !type )
// 		return SemaError("Typename '" + *type_name + "' not visible in current scope.");

// 	return TypeInstance::PointerTo(type, declarator->indirection_level);
// }

// struct sema_fn_proto_t
// {
// 	Type const* fn_type;
// 	Vec<TypedMember> values;
// };

// static SemaResult<sema_fn_proto_t>
// sema_fn_proto(Sema2& sema, ast::AstNode* node)
// {
// 	auto result = expected(sema, node, NodeType::FnProto);
// 	if( !result.ok() )
// 		return result;

// 	auto data = sema.query(node);
// 	auto mod = node->data.fn_proto;

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

// 	return (struct sema_fn_proto_t){
// 		.fn_type = fn_type,
// 		.values = params.unwrap() //
// 	};
// }

// Sema2::Sema2(ast::Ast& ast)
// 	: ast(ast)
// {
// 	scopes.reserve(100);

// 	scopes.emplace_back(&this->types);
// 	current_scope = &scopes.back();
// }

// SemaResult<TypeInstance>
// Sema2::sema(ast::AstNode* node)
// {
// 	switch( node->type )
// 	{
// 	case NodeType::Invalid:
// 		return SemaError("Invalid NodeType in Sema.");
// 	case NodeType::Module:
// 		return sema_module(node);
// 	case NodeType::Fn:
// 		return sema_fn(node);
// 	case NodeType::FnProto:
// 		break;
// 	case NodeType::FnParamList:
// 		break;
// 	case NodeType::ValueDecl:
// 		break;
// 	case NodeType::FnCall:
// 		return sema_fn_call(node);
// 	case NodeType::ExprList:
// 		break;
// 	case NodeType::Block:
// 		break;
// 	case NodeType::BinOp:
// 		break;
// 	case NodeType::Id:
// 		return sema_id(node);
// 	case NodeType::Assign:
// 		break;
// 	case NodeType::If:
// 		break;
// 	case NodeType::Let:
// 		break;
// 	case NodeType::Return:
// 		return sema_fn_return(node);
// 	case NodeType::Struct:
// 		break;
// 	case NodeType::MemberDef:
// 		break;
// 	case NodeType::While:
// 		break;
// 	case NodeType::For:
// 		break;
// 	case NodeType::StringLiteral:
// 		break;
// 	case NodeType::NumberLiteral:
// 		return sema_number_literal(node);
// 	case NodeType::TypeDeclarator:
// 		break;
// 	case NodeType::MemberAccess:
// 		break;
// 	case NodeType::Expr:
// 		return sema_expr(node);
// 	case NodeType::Stmt:
// 		return sema_stmt(node);
// 	}

// 	return SemaError("Unhandled ast NodeType in Sema.");
// }

// SemaResult<TypeInstance>
// Sema2::sema_module(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::Module);
// 	if( !result.ok() )
// 		return result;

// 	auto mod = node->data.mod;

// 	for( auto statement : mod.statements )
// 	{
// 		auto statement_result = sema(statement);
// 		// Don't really care about the sema result here.
// 		if( !statement_result.ok() )
// 		{
// 			return statement_result;
// 		}
// 	}

// 	return Ok();
// }

// SemaResult<TypeInstance>
// Sema2::sema_fn(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::Fn);
// 	if( !result.ok() )
// 		return result;

// 	auto mod = node->data.fn;
// 	auto data = ast.query<Sema2>(node);

// 	auto prototype = sema_fn_proto(*this, mod.prototype);
// 	if( !prototype.ok() )
// 		return prototype;

// 	if( mod.body )
// 	{
// 		auto parameters = prototype.unwrap().values;
// 		auto body = sema_block(mod.body, parameters);
// 		if( !body.ok() )
// 			return body;
// 	}

// 	return Ok();
// }

// SemaResult<TypeInstance>
// Sema2::sema_fn_block(ast::AstNode* node, Vec<TypedMember>& members)
// {}

// SemaResult<Vec<TypedMember>>
// Sema2::sema_fn_param_list(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::FnParamList);
// 	if( !result.ok() )
// 		return result;

// 	auto mod = node->data.fn_params;

// 	Vec<TypedMember> decls;

// 	for( auto param : mod.params )
// 	{
// 		auto value_decl = as_value_decl(*this, param);
// 		if( !value_decl.ok() )
// 			return value_decl;

// 		// Function params are delayed before adding to scope.
// 		auto tc = typecheck_value_decl(value_decl.unwrap());
// 		if( !tc.ok() )
// 			return tc;

// 		decls.emplace_back(tc.unwrap());
// 	}

// 	return decls;
// }

// // SemaResult<Type*>
// // Sema2::sema_value_decl(ast::AstNode* node)
// // {
// // 	auto result = expected(node, NodeType::ValueDecl);
// // 	if( !result.ok() )
// // 		return result;
// // }

// SemaResult<TypeInstance>
// Sema2::sema_fn_return(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::Return);
// 	if( !result.ok() )
// 		return result;

// 	auto mod = node->data.returnexpr;
// 	auto exprnode = sema(mod.expr);
// 	if( !exprnode.ok() )
// 		return exprnode;

// 	auto expected_type = *current_scope->get_expected_return();
// 	auto return_type = exprnode.unwrap();
// 	if( return_type != expected_type )
// 		return SemaError("Incorrect return type");

// 	// TODO: Return never here.
// 	return Ok();
// }

// SemaResult<TypeInstance>
// Sema2::sema_block(ast::AstNode* node, Vec<TypedMember>& members)
// {
// 	auto block_result = as_block(*this, node);
// 	if( !block_result.ok() )
// 		return block_result;

// 	auto block = block_result.unwrap();

// 	new_scope();

// 	for( auto member : members )
// 		add_value_identifier(member.name, member.type);

// 	for( auto stmts : block->statements )
// 	{
// 		auto stmt_result = sema(stmts);
// 		if( !stmt_result.ok() )
// 			return stmt_result;
// 	}

// 	pop_scope();

// 	return Ok();
// }

// SemaResult<TypeInstance>
// Sema2::sema_id(ast::AstNode* node)
// {
// 	auto result = as_id(*this, node);
// 	if( !result.ok() )
// 		return result;

// 	auto id_node = result.unwrap();
// 	auto type_name = id_node->name;

// 	auto type = current_scope->lookup_value_type(*type_name);
// 	if( !type )
// 		return SemaError("Name '" + *type_name + "' not visible in current scope.");

// 	return *type;
// }

// SemaResult<TypeInstance>
// Sema2::sema_fn_call(ast::AstNode* node)
// {
// 	auto result = as_fn_call(*this, node);
// 	if( !result.ok() )
// 		return result;

// 	auto fn_call = result.unwrap();

// 	auto expr_result = sema(fn_call->call_target);
// 	if( !expr_result.ok() )
// 		return expr_result;

// 	auto expr_type = expr_result.unwrap();
// 	if( !expr_type.type->is_function_type() )
// 		return SemaError("Expected function type.");

// 	return expr_type.type->get_return_type().value();
// }

// SemaResult<TypeInstance>
// Sema2::sema_type_decl(ast::AstNode* node)
// {
// 	auto result = as_type_decl(*this, node);
// 	if( !result.ok() )
// 		return result;
// }

// SemaResult<TypeInstance>
// Sema2::sema_stmt(ast::AstNode* node)
// {
// 	auto expr_result = as_stmt(*this, node);
// 	if( !expr_result.ok() )
// 		return expr_result;

// 	auto stmt = expr_result.unwrap();
// 	return sema(stmt->expr);
// }

// SemaResult<TypeInstance>
// Sema2::sema_number_literal(ast::AstNode* node)
// {
// 	return TypeInstance::OfType(types.i32_type());
// }

// SemaResult<TypeInstance>
// Sema2::sema_expr(ast::AstNode* node)
// {
// 	auto expr_result = as_expr(*this, node);
// 	if( !expr_result.ok() )
// 		return expr_result;

// 	auto stmt = expr_result.unwrap();
// 	return sema(stmt->expr);
// }

// SemaResult<TypedMember>
// Sema2::typecheck_value_decl(ast::AstValueDecl* decl)
// {
// 	auto type_decl = typecheck_type_decl(*this, decl->type_name);
// 	if( !type_decl.ok() )
// 		return type_decl;

// 	auto value_name_result = as_id(*this, decl->name);
// 	auto value_name = value_name_result.unwrap()->name;

// 	return TypedMember{type_decl.unwrap(), *value_name};
// }

// SemaResult<TypeInstance>
// Sema2::Ok()
// {
// 	return TypeInstance::OfType(types.void_type());
// }

// void
// Sema2::new_scope()
// {
// 	scopes.emplace_back(current_scope);
// 	current_scope = &scopes.back();
// }

// void
// Sema2::pop_scope()
// {
// 	current_scope->is_in_scope = false;
// 	current_scope = current_scope->get_parent();
// }

// void
// Sema2::add_value_identifier(String const& name, TypeInstance id)
// {
// 	return current_scope->add_value_identifier(name, id);
// }

// void
// Sema2::add_type_identifier(Type const* id)
// {
// 	return current_scope->add_type_identifier(id);
// }

// Type const*
// Sema2::CreateType(Type ty)
// {
// 	return types.define_type(ty);
// }

// SemaResult<TypeInstance>
// Sema2::expected(ast::AstNode* node, ast::NodeType type)
// {
// 	return ::expected(*this, node, type);
// }

// SemaTag*
// Sema2::query(ast::AstNode* node)
// {
// 	return ast.query<Sema2>(node);
// }