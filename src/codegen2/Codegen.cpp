#include "Codegen.h"

#include "ast2/AstCasts.h"

using namespace cg;
using namespace ast;

template<typename T>
CGResult<T>
expected(ast::AstNode* node, Cast<T> (*cast)(ast::AstNode* node))
{
	auto castr = cast(node);
	if( !castr.ok() )
		return CGError("Expected type '" + ast::to_string(node->type) + "'");

	return castr.unwrap();
}

CG::CG(ast::Ast& ast)
	: ast(ast)
{}

CGResult<CGed>
CG::codegen(ast::AstNode* node)
{
	switch( node->type )
	{
	case NodeType::Invalid:
		return CGError("Invalid NodeType in Codegen.");
	case NodeType::Module:
		return codegen_module(node);
	case NodeType::Fn:
		return codegen_fn(node);
	case NodeType::FnProto:
		break;
	case NodeType::FnParamList:
		break;
	case NodeType::ValueDecl:
		break;
	case NodeType::FnCall:
		return codegen_fn_call(node);
	case NodeType::ExprList:
		break;
	case NodeType::Block:
		break;
	case NodeType::BinOp:
		break;
	case NodeType::Id:
		return codegen_id(node);
	case NodeType::Assign:
		break;
	case NodeType::If:
		break;
	case NodeType::Let:
		break;
	case NodeType::Return:
		return codegen_fn_return(node);
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
		return codegen_number_literal(node);
	case NodeType::TypeDeclarator:
		break;
	case NodeType::MemberAccess:
		break;
	case NodeType::Expr:
		return codegen_expr(node);
	case NodeType::Stmt:
		return codegen_stmt(node);
	}

	return CGError("Unhandled ast NodeType in Codegen.");
}

CGResult<CGed>
CG::codegen_module(ast::AstNode* node)
{
	auto result = expected(node, NodeType::Module);
	if( !result.ok() )
		return result;

	auto mod = node->data.mod;

	for( auto statement : mod.statements )
	{
		auto statement_result = codegen(statement);
		// Don't really care about the sema result here.
		if( !statement_result.ok() )
			return statement_result;
	}

	return CGed{};
}

CGResult<CGed>
CG::codegen_fn(ast::AstNode* node)
{
	auto result = expected(node, NodeType::Fn);
	if( !result.ok() )
		return result;

	auto scope = get_scope(node);

	for( auto statement : mod.statements )
	{
		auto statement_result = codegen(statement);
		// Don't really care about the sema result here.
		if( !statement_result.ok() )
			return statement_result;
	}

	return CGed{};
}

CGResult<llvm::Type*>
codegen_fn_param(CG& cg, ast::AstNode* node)
{
	auto value_declr = ::expected(node, ast::as_value_decl);
	if( !value_declr.ok() )
		return value_declr;

	auto type_declr = ::expected(node, ast::as_type_decl);
	if( !type_declr.ok() )
		return type_declr;

	auto type_decl = type_declr.unwrap();
	auto type_name = type_decl.name;

	auto scope = cg.get_scope(node);
	// scope
}

CGResult<std::vector<llvm::Type*>>
codegen_fn_params(ast::AstNode* node)
{
	auto paramsr = ::expected(node, ast::as_fn_param_list);
	if( !paramsr.ok() )
		return paramsr;

	auto params = paramsr.unwrap();

	std::vector<llvm::Type*> ParamTys;
	for( auto param : params.params )
	{
		auto paramr = codegen_fn_param(param);
		if( !paramr.ok() )
			return paramr;

		ParamTys.push_back(ty);
	}

	return ParamTys;
}

CGResult<CGed>
CG::codegen_fn_proto(ast::AstNode* node)
{
	auto protor = ::expected(node, ast::as_fn_proto);
	if( !protor.ok() )
		return CGed{};

	auto scope = get_scope(node);

	auto proto = protor.unwrap();
	auto namesr = as_id(proto.name);
	if( !namesr.ok() )
		return namesr;

	auto name_node = namesr.unwrap();
	auto name = name_node.name;

	auto paramsr = codegen_fn_params(proto.params);
	if( !paramsr )
		return paramsr;

	auto ParamsTys = paramsr.unwrap();

	llvm::FunctionType* FT = FunctionType::get(ret_ty, ParamsTys, false);

	llvm::Function* Function =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, name, Module.get());
	scope->Function = Function;

	Functions.emplace(name, v)
}

CGResult<CGed>
CG::codegen_id(ast::AstNode* node)
{
	//
}

CGResult<CGed>
CG::codegen_block(ast::AstNode* node)
{
	auto blockr = ::expected(node, ast::as_block);
	if( !blockr.ok() )
		return CGed{};

	auto block = blockr.unwrap();

	for( auto stmt : block.statements )
	{
		auto stmtr = codegen(stmt);
		if( !stmtr.ok() )
			return stmtr;
	}

	return CGed{};
}

CGResult<CGed>
CG::codegen_fn_call(ast::AstNode* node)
{
	auto fn_callr = ::expected(node, ast::as_fn_call);
	if( !fn_callr.ok() )
		return CGed{};

	auto fn_call = fn_callr.unwrap();
	auto exprr = codegen_expr(fn_call.call_target);
	if( !exprr.ok() )
		return CGed{};

	const CallExprVal = exprr.unwrap();
}

CGResult<CGed>
CG::codegen_fn_return(ast::AstNode* node)
{
	auto fn_returnr = ::expected(node, ast::as_fn_return);
	if( !fn_returnr.ok() )
		return CGed{};
}

CGResult<CGed>
CG::codegen_number_literal(ast::AstNode* node)
{
	auto number_litr = ::expected(node, ast::as_number_literal);
	if( !number_litr.ok() )
		return CGed{};
}

CGResult<CGed>
CG::codegen_expr(ast::AstNode* node)
{
	auto exprr = ::expected(node, ast::as_expr);
	if( !exprr.ok() )
		return CGed{};
}

CGResult<CGed>
CG::codegen_stmt(ast::AstNode* node)
{
	auto stmtr = ::expected(node, ast::as_stmt);
	if( !stmtr.ok() )
		return CGed{};
}

CGScope*
CG::get_scope(ast::AstNode* node)
{
	auto sematag = ast.query<sema::Sema2>(node);

	auto s = sematag->scope;

	auto iter_scopes = scopes.find(s);
	if( iter_scopes != scopes.end() )
	{
		return &iter_scopes->second;
	}
	else
	{
		return nullptr;
	}
}

sema::Scope*
CG::get_sema_scope(ast::AstNode* node)
{
	auto sematag = ast.query<sema::Sema2>(node);
	return sematag->scope;
}