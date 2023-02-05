#pragma once
#include "IR.h"
#include "Scope.h"
#include "SemaResult.h"
#include "SemaTag.h"
#include "Types.h"
#include "ast2/Ast.h"
#include "ast2/AstNode.h"
#include "common/String.h"
#include "common/Vec.h"
#include "type/Type.h"

#include <optional>

namespace sema
{
class Sema2
{
	using TagType = SemaTag;

	Vec<Scope> scopes;
	Scope* current_scope = nullptr;

public:
	Types types;
	Sema2();
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

	Scope* push_scope();
	void pop_scope();
	void add_value_identifier(String const& name, TypeInstance id);
	void add_type_identifier(Type const* id);

	std::optional<TypeInstance> get_expected_return();
	void set_expected_return(TypeInstance id);
	void clear_expected_return();

	Type const* CreateType(Type ty);

	// SemaResult<TypeInstance> expected(ast::AstNode* node, ast::NodeType type);

	std::optional<TypeInstance> lookup_name(String const& name);
	Type const* lookup_type(String const& name);

	Vec<ir::IRTopLevelStmt*>* create_tlslist();
	Vec<ir::IRStmt*>* create_slist();
	Vec<ir::IRExpr*>* create_elist();
	Vec<ir::IRValueDecl*>* create_argslist();
	String* create_name(char const* s, int size);

	ir::IRModule* Module(ast::AstNode* node, Vec<ir::IRTopLevelStmt*>* stmts);
	ir::IRTopLevelStmt* TLS(ir::IRExternFn*);
	ir::IRTopLevelStmt* TLS(ir::IRFunction*);
	ir::IRFunction* Fn(ast::AstNode* node, ir::IRProto* proto, ir::IRBlock* block);
	ir::IRCall* FnCall(ast::AstNode* node, ir::IRExpr* call_target, ir::IRArgs* args);
	ir::IRExternFn* ExternFn(ast::AstNode* node, ir::IRProto* stmts);
	ir::IRProto* Proto(
		ast::AstNode* node,
		String* name,
		Vec<ir::IRValueDecl*>* args,
		ir::IRTypeDeclaraor* rt,
		Type const* fn_type);
	ir::IRBlock* Block(ast::AstNode* node, Vec<ir::IRStmt*>* stmts);
	ir::IRReturn* Return(ast::AstNode* node, ir::IRExpr* expr);
	ir::IRValueDecl* ValueDecl(ast::AstNode* node, String* name, ir::IRTypeDeclaraor* rt);
	ir::IRTypeDeclaraor* TypeDecl(ast::AstNode* node, sema::TypeInstance type);
	ir::IRExpr* Expr(ir::IRCall*);
	ir::IRExpr* Expr(ir::IRNumberLiteral*);
	ir::IRExpr* Expr(ir::IRStringLiteral*);
	ir::IRExpr* Expr(ir::IRId*);
	ir::IRExpr* Expr(ir::IRValueDecl*);
	ir::IRStmt* Stmt(ir::IRReturn*);
	ir::IRStmt* Stmt(ir::IRExpr*);
	ir::IRStmt* Stmt(ir::IRLet*);
	ir::IRStmt* Stmt(ir::IRAssign*);
	ir::IRArgs* Args(ast::AstNode* node, Vec<ir::IRExpr*>* args);
	ir::IRNumberLiteral*
	NumberLiteral(ast::AstNode* node, TypeInstance type_instance, long long val);
	ir::IRStringLiteral*
	StringLiteral(ast::AstNode* node, TypeInstance type_instance, String* name);
	ir::IRId* Id(ast::AstNode* node, String* name, sema::TypeInstance type);
	ir::IRLet* Let(ast::AstNode* node, String* name, ir::IRAssign* assign);
	ir::IRAssign* Assign(ast::AstNode* node, ast::AssignOp op, ir::IRExpr* lhs, ir::IRExpr* rhs);
};

} // namespace sema
