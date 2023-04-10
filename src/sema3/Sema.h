#pragma once

#include "Hir.h"
#include "HirNode.h"
#include "SemaResult.h"
#include "SymBuiltins.h"
#include "SymTab.h"
#include "Types.h"

#include <map>

class Sema
{
	Hir& hir;
	Types& types;
	SymTab& sym_tab;

	SymBuiltins builtins;

public:
	Sema(Hir& hir, Types& types, SymTab& sym_tab);

	SemaResult<HirNode*> sema_module(AstNode* ast_module);
	SemaResult<HirNode*> sema_module_stmt_any(AstNode* ast_module_stmt);
	SemaResult<HirNode*> sema_func(AstNode* ast_func);
	SemaResult<HirNode*> sema_func_proto(AstNode* ast_func_proto);

	SemaResult<HirNode*> sema_block(AstNode* ast_func);
	SemaResult<HirNode*> sema_struct(AstNode* ast_struct);
	SemaResult<HirNode*> sema_union(AstNode* ast_union);
	SemaResult<HirNode*> sema_enum(AstNode* ast_enum);
	SemaResult<HirNode*> sema_stmt(AstNode* ast_stmt);
	SemaResult<HirNode*> sema_stmt_any(AstNode* ast_stmt);
	SemaResult<HirNode*> sema_return(AstNode* ast_return);
	SemaResult<HirNode*> sema_sizeof(AstNode* ast_sizeof);
	SemaResult<HirNode*> sema_addressof(AstNode* ast_addressof);
	SemaResult<HirNode*> sema_boolnot(AstNode* ast_boolnot);
	SemaResult<HirNode*> sema_array_access(AstNode* ast_boolnot);
	SemaResult<HirNode*> sema_member_access(AstNode* ast_boolnot);
	SemaResult<HirNode*> sema_let(AstNode* ast_let);
	SemaResult<HirNode*> sema_if(AstNode* ast_let);
	SemaResult<HirNode*> sema_expr(AstNode* ast_expr);
	SemaResult<HirNode*> sema_expr_any(AstNode* ast_expr);
	SemaResult<HirNode*> sema_id(AstNode* ast_id);
	SemaResult<HirNode*> sema_func_call(AstNode* ast_func_call);
	SemaResult<HirNode*> sema_bin_op(AstNode* ast_bin_op);
	SemaResult<HirNode*> sema_number_literal(AstNode* ast_number_literal);

private:
	// Checks if two types can be equal after coercing the subject type.
	SemaResult<QualifiedTy> type_declarator(AstNode* ast_type_declarator);
	SemaResult<std::map<std::string, QualifiedTy>> decl_list(std::vector<AstNode*>& ast_decls);
	SemaResult<HirNode*> equal_coercion(QualifiedTy target, HirNode* node);
};