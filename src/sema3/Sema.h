#pragma once

#include "Hir.h"
#include "HirNode.h"
#include "SemaResult.h"
#include "SymLookup.h"
#include "SymTab.h"
#include "Types.h"

class Sema
{
	Hir& hir;
	Types& types;
	SymTab& sym_tab;

	SymLookup lookup;

public:
	Sema(Hir& hir, Types& types, SymTab& sym_tab);

	SemaResult<HirNode*> sema_module(AstNode* ast_module);
	SemaResult<HirNode*> sema_module_stmt_any(AstNode* ast_module_stmt);
	SemaResult<HirNode*> sema_func(AstNode* ast_func);
	SemaResult<HirNode*> sema_func_proto(AstNode* ast_func_proto);
	SemaResult<HirNode*> sema_type_declarator(AstNode* ast_type_declarator);
	SemaResult<HirNode*> sema_block(AstNode* ast_func);
	SemaResult<HirNode*> sema_stmt(AstNode* ast_stmt);
	SemaResult<HirNode*> sema_return(AstNode* ast_return);
	SemaResult<HirNode*> sema_expr(AstNode* ast_expr);
	SemaResult<HirNode*> sema_number_literal(AstNode* ast_number_literal);

private:
};