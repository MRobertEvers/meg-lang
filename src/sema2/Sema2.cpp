
#include "Sema2.h"

#include <iostream>

using namespace ast;
using namespace sema;

static SemaResult<TypeInstance>
expected(Sema2& sema, ast::AstNode* node, ast::NodeType type)
{
	if( node->type != type )
		return SemaError("Expected type '" + to_string(type) + "'");

	auto semtag = sema.query(node);
	semtag->scope = sema.current_scope;
	semtag->tag_type = SemaTagType::Sema;

	return TypeInstance::OfType(sema.types.void_type());
}

static SemaResult<AstId*>
as_id(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstId::nt);
	if( !result.ok() )
		return result;

	return &node->data.id;
}

static SemaResult<AstBlock*>
as_block(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstBlock::nt);
	if( !result.ok() )
		return result;

	return &node->data.block;
}

static SemaResult<AstTypeDeclarator*>
as_type_decl(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstTypeDeclarator::nt);
	if( !result.ok() )
		return result;

	return &node->data.type_declarator;
}

static SemaResult<AstValueDecl*>
as_value_decl(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstValueDecl::nt);
	if( !result.ok() )
		return result;

	return &node->data.value_decl;
}

static SemaResult<AstFnCall*>
as_fn_call(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstFnCall::nt);
	if( !result.ok() )
		return result;

	return &node->data.fn_call;
}

static SemaResult<AstStmt*>
as_stmt(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstStmt::nt);
	if( !result.ok() )
		return result;

	return &node->data.stmt;
}

static SemaResult<AstExpr*>
as_expr(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, AstExpr::nt);
	if( !result.ok() )
		return result;

	return &node->data.expr;
}

static SemaResult<TypeInstance>
typecheck_type_decl(Sema2& sema, ast::AstNode* node)
{
	auto type_name_result = as_type_decl(sema, node);
	if( !type_name_result.ok() )
		return type_name_result;

	// static_assert(type_name_result.unwrap()->classification == IdClassification::TypeIdentifier);
	auto declarator = type_name_result.unwrap();
	auto type_name = declarator->name;

	auto type = sema.current_scope->lookup_type(*type_name);
	if( !type )
		return SemaError("Typename '" + *type_name + "' not visible in current scope.");

	return TypeInstance::PointerTo(type, declarator->indirection_level);
}

struct sema_fn_proto_t
{
	Type const* fn_type;
	Vec<TypedMember> values;
};

static SemaResult<sema_fn_proto_t>
sema_fn_proto(Sema2& sema, ast::AstNode* node)
{
	auto result = expected(sema, node, NodeType::FnProto);
	if( !result.ok() )
		return result;

	auto data = sema.query(node);
	auto mod = node->data.fn_proto;

	auto idnode = as_id(sema, mod.name);
	if( !idnode.ok() )
		return result;

	auto fn_name = idnode.unwrap()->name;

	auto params = sema.sema_fn_param_list(mod.params);
	if( !params.ok() )
		return params;

	auto retnode = typecheck_type_decl(sema, mod.return_type);
	if( !retnode.ok() )
		return retnode;

	auto return_type = retnode.unwrap();

	sema.current_scope->set_expected_return(return_type);

	auto newtype = Type::Function(*fn_name, params.unwrap(), return_type);
	auto fn_type = sema.CreateType(newtype);
	sema.add_type_identifier(fn_type);
	sema.add_value_identifier(*fn_name, TypeInstance::OfType(fn_type));

	return (struct sema_fn_proto_t){
		.fn_type = fn_type,
		.values = params.unwrap() //
	};
}

Sema2::Sema2(ast::Ast& ast)
	: ast(ast)
{
	scopes.reserve(100);

	scopes.emplace_back(&this->types);
	current_scope = &scopes.back();
}

SemaResult<TypeInstance>
Sema2::sema(ast::AstNode* node)
{
	switch( node->type )
	{
	case NodeType::Invalid:
		return SemaError("Invalid NodeType in Sema.");
	case NodeType::Module:
		return sema_module(node);
	case NodeType::Fn:
		return sema_fn(node);
	case NodeType::FnProto:
		break;
	case NodeType::FnParamList:
		break;
	case NodeType::ValueDecl:
		break;
	case NodeType::FnCall:
		return sema_fn_call(node);
	case NodeType::ExprList:
		break;
	case NodeType::Block:
		break;
	case NodeType::BinOp:
		break;
	case NodeType::Id:
		return sema_id(node);
	case NodeType::Assign:
		break;
	case NodeType::If:
		break;
	case NodeType::Let:
		break;
	case NodeType::Return:
		return sema_fn_return(node);
	case NodeType::Struct:
		break;
	case NodeType::MemberDef:
		break;
	case NodeType::While:
		break;
	case NodeType::For:
		break;
	case NodeType::StringLiteral:
		break;
	case NodeType::NumberLiteral:
		return sema_number_literal(node);
	case NodeType::TypeDeclarator:
		break;
	case NodeType::MemberAccess:
		break;
	case NodeType::Expr:
		return sema_expr(node);
	case NodeType::Stmt:
		return sema_stmt(node);
	}

	return SemaError("Unhandled ast NodeType in Sema.");
}

SemaResult<TypeInstance>
Sema2::sema_module(ast::AstNode* node)
{
	auto result = expected(node, NodeType::Module);
	if( !result.ok() )
		return result;

	auto mod = node->data.mod;

	for( auto statement : mod.statements )
	{
		auto statement_result = sema(statement);
		// Don't really care about the sema result here.
		if( !statement_result.ok() )
		{
			return statement_result;
		}
	}

	return Ok();
}

SemaResult<TypeInstance>
Sema2::sema_fn(ast::AstNode* node)
{
	auto result = expected(node, NodeType::Fn);
	if( !result.ok() )
		return result;

	auto mod = node->data.fn;
	auto data = ast.query<Sema2>(node);

	auto prototype = sema_fn_proto(*this, mod.prototype);
	if( !prototype.ok() )
		return prototype;

	if( mod.body )
	{
		auto parameters = prototype.unwrap().values;
		auto body = sema_block(mod.body, parameters);
		if( !body.ok() )
			return body;
	}

	return Ok();
}

SemaResult<TypeInstance>
Sema2::sema_fn_block(ast::AstNode* node, Vec<TypedMember>& members)
{}

SemaResult<Vec<TypedMember>>
Sema2::sema_fn_param_list(ast::AstNode* node)
{
	auto result = expected(node, NodeType::FnParamList);
	if( !result.ok() )
		return result;

	auto mod = node->data.fn_params;

	Vec<TypedMember> decls;

	for( auto param : mod.params )
	{
		auto value_decl = as_value_decl(*this, param);
		if( !value_decl.ok() )
			return value_decl;

		// Function params are delayed before adding to scope.
		auto tc = typecheck_value_decl(value_decl.unwrap());
		if( !tc.ok() )
			return tc;

		decls.emplace_back(tc.unwrap());
	}

	return decls;
}

// SemaResult<Type*>
// Sema2::sema_value_decl(ast::AstNode* node)
// {
// 	auto result = expected(node, NodeType::ValueDecl);
// 	if( !result.ok() )
// 		return result;
// }

SemaResult<TypeInstance>
Sema2::sema_fn_return(ast::AstNode* node)
{
	auto result = expected(node, NodeType::Return);
	if( !result.ok() )
		return result;

	auto mod = node->data.returnexpr;
	auto exprnode = sema(mod.expr);
	if( !exprnode.ok() )
		return exprnode;

	auto expected_type = *current_scope->get_expected_return();
	auto return_type = exprnode.unwrap();
	if( return_type != expected_type )
		return SemaError("Incorrect return type");

	// TODO: Return never here.
	return Ok();
}

SemaResult<TypeInstance>
Sema2::sema_block(ast::AstNode* node, Vec<TypedMember>& members)
{
	auto block_result = as_block(*this, node);
	if( !block_result.ok() )
		return block_result;

	auto block = block_result.unwrap();

	new_scope();

	for( auto member : members )
		add_value_identifier(member.name, member.type);

	for( auto stmts : block->statements )
	{
		auto stmt_result = sema(stmts);
		if( !stmt_result.ok() )
			return stmt_result;
	}

	pop_scope();

	return Ok();
}

SemaResult<TypeInstance>
Sema2::sema_id(ast::AstNode* node)
{
	auto result = as_id(*this, node);
	if( !result.ok() )
		return result;

	auto id_node = result.unwrap();
	auto type_name = id_node->name;

	auto type = current_scope->lookup_value_type(*type_name);
	if( !type )
		return SemaError("Name '" + *type_name + "' not visible in current scope.");

	return *type;
}

SemaResult<TypeInstance>
Sema2::sema_fn_call(ast::AstNode* node)
{
	auto result = as_fn_call(*this, node);
	if( !result.ok() )
		return result;

	auto fn_call = result.unwrap();

	auto expr_result = sema(fn_call->call_target);
	if( !expr_result.ok() )
		return expr_result;

	auto expr_type = expr_result.unwrap();
	if( !expr_type.type->is_function_type() )
		return SemaError("Expected function type.");

	return expr_type.type->get_return_type().value();
}

SemaResult<TypeInstance>
Sema2::sema_type_decl(ast::AstNode* node)
{
	auto result = as_type_decl(*this, node);
	if( !result.ok() )
		return result;
}

SemaResult<TypeInstance>
Sema2::sema_stmt(ast::AstNode* node)
{
	auto expr_result = as_stmt(*this, node);
	if( !expr_result.ok() )
		return expr_result;

	auto stmt = expr_result.unwrap();
	return sema(stmt->expr);
}

SemaResult<TypeInstance>
Sema2::sema_number_literal(ast::AstNode* node)
{
	return TypeInstance::OfType(types.i32_type());
}

SemaResult<TypeInstance>
Sema2::sema_expr(ast::AstNode* node)
{
	auto expr_result = as_expr(*this, node);
	if( !expr_result.ok() )
		return expr_result;

	auto stmt = expr_result.unwrap();
	return sema(stmt->expr);
}

SemaResult<TypedMember>
Sema2::typecheck_value_decl(ast::AstValueDecl* decl)
{
	auto type_decl = typecheck_type_decl(*this, decl->type_name);
	if( !type_decl.ok() )
		return type_decl;

	auto value_name_result = as_id(*this, decl->name);
	auto value_name = value_name_result.unwrap()->name;

	return TypedMember{type_decl.unwrap(), *value_name};
}

SemaResult<TypeInstance>
Sema2::Ok()
{
	return TypeInstance::OfType(types.void_type());
}

void
Sema2::new_scope()
{
	scopes.emplace_back(current_scope);
	current_scope = &scopes.back();
}

void
Sema2::pop_scope()
{
	current_scope->is_in_scope = false;
	current_scope = current_scope->get_parent();
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

Type const*
Sema2::CreateType(Type ty)
{
	return types.define_type(ty);
}

SemaResult<TypeInstance>
Sema2::expected(ast::AstNode* node, ast::NodeType type)
{
	return ::expected(*this, node, type);
}

SemaTag*
Sema2::query(ast::AstNode* node)
{
	return ast.query<Sema2>(node);
}