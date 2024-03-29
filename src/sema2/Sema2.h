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

#include <map>
#include <optional>

namespace sema
{

struct SwitchContext
{
	TypeInstance cond_expr_type;

	SwitchContext(TypeInstance cond)
		: cond_expr_type(cond)
	{}
};

class Sema2
{
	using TagType = SemaTag;

	Vec<Scope> scopes;
	Scope* current_scope = nullptr;

	// We must track the current module so we can emit
	// generated functions.

	Vec<ir::IRTopLevelStmt*> generated;

	std::optional<SwitchContext> switch_context_;

public:
	Types types;
	Sema2();

	Scope* push_scope();
	void pop_scope();
	void add_value_identifier(String const& name, TypeInstance id);
	void add_type_identifier(Type const* id);

	std::optional<TypeInstance> get_expected_return();
	void set_expected_return(TypeInstance id);
	void clear_expected_return();

	std::optional<SwitchContext> switch_context() { return switch_context_; }
	void switch_context_set(std::optional<SwitchContext> ctx) { switch_context_ = ctx; }
	void switch_context_clear() { switch_context_.reset(); }

	Type* CreateType(Type ty);

	// SemaResult<TypeInstance> expected(ast::AstNode* node, ast::NodeType type);

	std::optional<TypeInstance> lookup_name(String const& name);
	Type const* lookup_type(String const& name);

	Vec<ir::IRDesignator*>* create_designator_list();
	Vec<ir::IRTopLevelStmt*>* create_tlslist();
	std::map<String, ir::IREnumMember*>* create_enum_member_map();
	std::map<String, ir::IRValueDecl*>* create_member_map();
	std::map<String, ir::IRExpr*>* create_expr_map();
	Vec<ir::IRStmt*>* create_slist();
	Vec<ir::IRExpr*>* create_elist();
	Vec<ir::IRParam*>* create_argslist();
	String* create_name(char const* s, int size);

	ir::IRModule* Module(ast::AstNode* node, Vec<ir::IRTopLevelStmt*>* stmts);
	ir::IRTopLevelStmt* TLS(ir::IRExternFn*);
	ir::IRTopLevelStmt* TLS(ir::IRFunction*);
	ir::IRTopLevelStmt* TLS(ir::IRStruct*);
	ir::IRTopLevelStmt* TLS(ir::IRUnion*);
	ir::IRTopLevelStmt* TLS(ir::IREnum*);
	ir::IRFunction* Fn(ast::AstNode* node, ir::IRProto* proto, ir::IRBlock* block);
	ir::IRCall* FnCall(ast::AstNode* node, ir::IRExpr* call_target, ir::IRArgs* args);
	ir::IRExternFn* ExternFn(ast::AstNode* node, ir::IRProto* stmts);
	ir::IRProto* Proto(
		ast::AstNode* node,
		String* name,
		Vec<ir::IRParam*>* args,
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
	ir::IRExpr* Expr(ir::IRBinOp*);
	ir::IRExpr* Expr(ir::IRMemberAccess*);
	ir::IRExpr* Expr(ir::IRIndirectMemberAccess*);
	ir::IRExpr* Expr(ir::IRAddressOf*);
	ir::IRExpr* Expr(ir::IRDeref*);
	ir::IRExpr* Expr(ir::IRArrayAccess*);
	ir::IRExpr* Expr(ir::IREmpty*);
	ir::IRExpr* Expr(ir::IRIs*);
	ir::IRExpr* Expr(ir::IRInitializer*);
	ir::IRStmt* Stmt(ir::IRReturn*);
	ir::IRStmt* Stmt(ir::IRExpr*);
	ir::IRStmt* Stmt(ir::IRLet*);
	ir::IRStmt* Stmt(ir::IRAssign*);
	ir::IRStmt* Stmt(ir::IRIf*);
	ir::IRStmt* Stmt(ir::IRFor*);
	ir::IRStmt* Stmt(ir::IRWhile*);
	ir::IRStmt* Stmt(ir::IRElse*);
	ir::IRStmt* Stmt(ir::IRBlock*);
	ir::IRStmt* Stmt(ir::IRSwitch*);
	ir::IRStmt* Stmt(ir::IRCase*);
	ir::IRArgs* Args(ast::AstNode* node, Vec<ir::IRExpr*>* args);
	ir::IRNumberLiteral*
	NumberLiteral(ast::AstNode* node, TypeInstance type_instance, long long val);
	ir::IRStringLiteral*
	StringLiteral(ast::AstNode* node, TypeInstance type_instance, String* name);
	ir::IRId*
	Id(ast::AstNode* node, Vec<String*>* name_parts, sema::TypeInstance type, bool is_type_id);
	ir::IRSwitch* Switch(ast::AstNode* node, ir::IRExpr* expr, ir::IRBlock* block);
	ir::IRCase* CaseDefault(ast::AstNode* node, ir::IRStmt* stmt);
	ir::IRCase* Case(ast::AstNode* node, long long expr, ir::IRStmt* stmt);
	ir::IRCase* Case(ast::AstNode* node, long long expr, ir::IRStmt* stmt, Vec<ir::IRParam*>* args);
	ir::IRLet* Let(ast::AstNode* node, String* name, ir::IRAssign* assign);
	ir::IRIs*
	Is(ast::AstNode* node,
	   ir::IRExpr* expr,
	   ir::IRTypeDeclaraor* type_decl,
	   sema::TypeInstance bool_type);
	ir::IRLet* LetEmpty(ast::AstNode* node, String* name, sema::TypeInstance type);
	ir::IRIf* If(ast::AstNode* node, ir::IRExpr* bool_expr, ir::IRStmt*, ir::IRElse*);
	ir::IRIf*
	IfArrow(ast::AstNode*, ir::IRExpr*, ir::IRStmt*, ir::IRElse*, Vec<ir::IRParam*>* args);
	ir::IRElse* Else(ast::AstNode* node, ir::IRStmt* assign);
	ir::IRAssign* Assign(ast::AstNode*, ast::AssignOp, ir::IRExpr*, ir::IRExpr*);
	ir::IRBinOp* BinOp(ast::AstNode*, ast::BinOp, ir::IRExpr*, ir::IRExpr*, TypeInstance);
	ir::IRStruct* Struct(ast::AstNode*, sema::Type const*, std::map<String, ir::IRValueDecl*>*);
	ir::IRUnion* Union(ast::AstNode*, sema::Type const*, std::map<String, ir::IRValueDecl*>*);
	ir::IREnum* Enum(ast::AstNode*, sema::Type const*, std::map<String, ir::IREnumMember*>*);
	ir::IREnumMember* EnumMemberStruct(
		ast::AstNode*, sema::Type const*, ir::IRStruct*, String* name, EnumNominal idx);
	ir::IREnumMember* EnumMemberId(ast::AstNode*, sema::Type const*, String* name, EnumNominal idx);
	ir::IRMemberAccess*
	MemberAccess(ast::AstNode*, ir::IRExpr* expr, sema::MemberTypeInstance, String*);
	ir::IRIndirectMemberAccess*
	IndirectMemberAccess(ast::AstNode*, ir::IRExpr* expr, sema::MemberTypeInstance, String*);
	ir::IRVarArg* VarArg(ast::AstNode*);
	ir::IRAddressOf* AddressOf(ast::AstNode*, ir::IRExpr* expr, sema::TypeInstance);
	ir::IRDeref* Deref(ast::AstNode*, ir::IRExpr* expr, sema::TypeInstance);
	ir::IREmpty* Empty(ast::AstNode*, sema::TypeInstance);
	ir::IRArrayAccess*
	ArrayAcess(ast::AstNode*, ir::IRExpr* array_target, ir::IRExpr* expr, sema::TypeInstance);
	ir::IRParam* IRParam(ast::AstNode*, ir::IRValueDecl* decl);
	ir::IRParam* IRParam(ast::AstNode*, ir::IRVarArg* var_arg);
	ir::IRFor*
	For(ast::AstNode*, ir::IRExpr* condition, ir::IRStmt* init, ir::IRStmt* end, ir::IRStmt* body);
	ir::IRWhile* While(ast::AstNode*, ir::IRExpr* condition, ir::IRStmt* body);
	ir::IRInitializer*
	Initializer(ast::AstNode*, String*, Vec<ir::IRDesignator*>*, sema::TypeInstance);
	ir::IRDesignator* Designator(ast::AstNode*, sema::MemberTypeInstance, ir::IRExpr* expr);
};

} // namespace sema
