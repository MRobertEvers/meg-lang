#pragma once
#include "IR.h"
#include "Scope.h"
#include "SemaResult.h"
#include "SemaTag.h"
#include "Types.h"
#include "ast2/Ast.h"
#include "ast2/AstNode.h"
#include "common/Vec.h"
#include "type/Type.h"

namespace sema
{
class Sema2
{
	using TagType = SemaTag;

public:
	// Types types;
	// Vec<Scope> scopes;
	// Scope* current_scope = nullptr;

	// SemaTag* query(ast::AstNode* node);

	// SemaResult<ir::IRModule> sema(ast::AstNode* node);

	// SemaResult<TypeInstance> sema_module(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_fn(ast::AstNode* node);
	// SemaResult<Vec<TypedMember>> sema_fn_param_list(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_fn_param_decl(ast::AstNode* node);

	// SemaResult<TypeInstance> sema_fn_return(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_fn_block(ast::AstNode* node, Vec<TypedMember>& members);
	// SemaResult<TypeInstance> sema_block(ast::AstNode* node, Vec<TypedMember>& members);
	// SemaResult<TypeInstance> sema_id(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_type_decl(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_fn_call(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_number_literal(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_stmt(ast::AstNode* node);
	// SemaResult<TypeInstance> sema_expr(ast::AstNode* node);

	// SemaResult<TypedMember> typecheck_value_decl(ast::AstValueDecl* decl);

	// SemaResult<TypeInstance> Ok();

	// void new_scope();
	// void pop_scope();
	// void add_value_identifier(String const& name, TypeInstance id);
	// void add_type_identifier(Type const* id);

	// Type const* CreateType(Type ty);

	// SemaResult<TypeInstance> expected(ast::AstNode* node, ast::NodeType type);

	Vec<ir::IRTopLevelStmt*>* create_tlslist();
	Vec<ir::IRStmt*>* create_slist();

	ir::IRModule* Module(ast::AstNode* node, Vec<ir::IRTopLevelStmt*>* stmts);
};

} // namespace sema
