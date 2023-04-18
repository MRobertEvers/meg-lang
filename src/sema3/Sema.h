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
	Ast& ast;
	Hir& hir;
	Types& types;
	SymTab& sym_tab;

	SymBuiltins builtins;

	// Used during template instantiation to inject top level statements.
	std::vector<HirNode*>* current_module = nullptr;
	HirFuncProto* current_function = nullptr;

public:
	Sema(Ast& ast, Hir& hir, Types& types, SymTab& sym_tab);

	SemaResult<HirNode*> sema_module(AstNode* ast_module);
	SemaResult<HirNode*> sema_module_stmt_any(AstNode* ast_module_stmt);
	SemaResult<HirNode*> sema_func(AstNode* ast_func);
	SemaResult<HirNode*> sema_func_proto(AstNode* ast_func_proto);
	SemaResult<HirNode*> sema_template(AstNode* ast_template);
	SemaResult<HirNode*> sema_block(AstNode* ast_func);
	SemaResult<HirNode*> sema_struct(AstNode* ast_struct);
	SemaResult<HirNode*> sema_union(AstNode* ast_union);
	SemaResult<HirNode*> sema_enum(AstNode* ast_enum);
	SemaResult<HirNode*> sema_interface(AstNode* ast_interface);
	SemaResult<HirNode*> sema_interface_member(AstNode* ast_node);
	SemaResult<HirNode*> sema_stmt(AstNode* ast_stmt);
	SemaResult<HirNode*> sema_stmt_any(AstNode* ast_stmt);
	SemaResult<HirNode*> sema_return(AstNode* ast_return);
	SemaResult<HirNode*> sema_sizeof(AstNode* ast_sizeof);
	SemaResult<HirNode*> sema_addressof(AstNode* ast_addressof);
	SemaResult<HirNode*> sema_boolnot(AstNode* ast_boolnot);
	SemaResult<HirNode*> sema_assign(AstNode* ast_assign);
	SemaResult<HirNode*> sema_array_access(AstNode* ast_array_access);
	SemaResult<HirNode*> sema_member_access(AstNode* ast_member_access);
	SemaResult<HirNode*> sema_let(AstNode* ast_let);
	SemaResult<HirNode*> sema_if(AstNode* ast_if);
	SemaResult<HirNode*> sema_initializer(AstNode* ast_initializer);
	SemaResult<HirNode*> sema_is(AstNode* ast_is);
	SemaResult<HirNode*> sema_using(AstNode* ast_using);
	SemaResult<HirNode*> sema_expr(AstNode* ast_expr);
	SemaResult<HirNode*> sema_expr_any(AstNode* ast_expr);
	SemaResult<HirSwitch::CondThen> sema_case(AstNode* ast_case);
	SemaResult<HirNode*> sema_default(AstNode* ast_default);
	SemaResult<HirNode*> sema_yield(AstNode* ast_yield);
	SemaResult<HirNode*> sema_id(AstNode* ast_id);
	SemaResult<HirNode*> sema_for(AstNode* ast_for);
	SemaResult<HirNode*> sema_while(AstNode* ast_while);
	SemaResult<HirNode*> sema_switch(AstNode* ast_switch);
	SemaResult<HirNode*> sema_func_call(AstNode* ast_func_call);
	SemaResult<HirNode*> sema_bin_op(AstNode* ast_bin_op);
	SemaResult<HirNode*> sema_bin_op_long(AstNode* ast_bin_op);
	SemaResult<HirNode*> sema_bin_op_short_circuit(AstNode* ast_bin_op);
	SemaResult<HirNode*> sema_number_literal(AstNode* ast_number_literal);

private:
	struct TypeDeclResult
	{
		Sym* sym;
		QualifiedTy qty;
	};
	SemaResult<TypeDeclResult> type_declarator(AstNode* ast_type_declarator);
	SemaResult<std::map<std::string, QualifiedTy>> decl_list(std::vector<AstNode*>& ast_decls);
	// Checks if two types can be equal after coercing the subject type.
	SemaResult<HirNode*> equal_coercion(QualifiedTy target, HirNode* node);
	SemaResult<Sym*> lookup_or_instantiate_template(AstNode* ast_template_id);
};