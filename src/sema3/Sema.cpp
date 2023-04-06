#include "Sema.h"

#include <string>
#include <vector>

static SemaError
NotImpl()
{
	return SemaError("Not Implemented.");
}

Sema::Sema(Hir& hir, Types& types, SymTab& sym_tab)
	: hir(hir)
	, types(types)
	, sym_tab(sym_tab)
{
	Ty const* ty = types.create<TyPrimitive>("i32");
	sym_tab.create_named<SymType>("i32", ty);
}

SemaResult<HirNode*>
Sema::sema_module(AstNode* ast_module)
{
	std::vector<HirNode*> statements;

	AstModule& mod = ast_module->data.ast_module;

	for( auto& stmt : mod.statements )
	{
		auto stmt_result = sema_module_stmt_any(stmt);
		if( !stmt_result.ok() )
			return stmt_result;

		statements.push_back(stmt_result.unwrap());
	}

	return hir.create<HirModule>(statements);
}

SemaResult<HirNode*>
Sema::sema_module_stmt_any(AstNode* ast_stmt)
{
	switch( ast_stmt->kind )
	{
	case NodeKind::Func:
		return sema_func(ast_stmt);
	default:
		return NotImpl();
	}
}

SemaResult<HirNode*>
Sema::sema_func(AstNode* ast_func)
{
	AstFunc& func = ast_func->data.ast_func;
	auto proto_result = sema_func_proto(func.proto);
	if( !proto_result.ok() )
		return proto_result;

	auto body_result = sema_block(func.body);
	if( !body_result.ok() )
		return body_result;

	return hir.create<HirFunc>(proto_result.unwrap(), body_result.unwrap());
}

static HirFuncProto::Linkage
translate_linkage(AstFuncProto::Linkage ast_linkage)
{
	switch( ast_linkage )
	{
	case AstFuncProto::Linkage::Extern:
		return HirFuncProto::Linkage::Extern;
	default:
		return HirFuncProto::Linkage::None;
	}
}

SemaResult<HirNode*>
Sema::sema_func_proto(AstNode* ast_func_proto)
{
	AstFuncProto& func_proto = ast_func_proto->data.ast_func_proto;

	HirFuncProto::Linkage linkage = translate_linkage(func_proto.linkage);

	// TODO: Simple names only?
	AstId& id = func_proto.id->data.ast_id;
	HirNode* hir_id = hir.create<HirId>(id.name_parts);

	std::vector<HirNode*> parameters;

	auto rt_type_declarator_result = sema_type_declarator(func_proto.rt_type_declarator);
	if( !rt_type_declarator_result.ok() )
		return rt_type_declarator_result;

	// The function type does not need to go into the symbol table.
	// Presumably we just look up the function symbol and get the type
	// from that??
	// TODO: Enter the function in the symbol table
	// The function in the symbol table acts as an overload set
	Ty const* func_ty = types.create<TyFunc>(id.name_parts.parts[0]);

	sym_tab.create_named<SymFunc>(id.name_parts.parts[0], func_ty);

	return hir.create<HirFuncProto>(
		linkage, hir_id, parameters, rt_type_declarator_result.unwrap(), func_ty);
}

SemaResult<HirNode*>
Sema::sema_type_declarator(AstNode* ast_type_declarator)
{
	AstTypeDeclarator& type_declarator = ast_type_declarator->data.ast_type_declarator;

	AstId& id = type_declarator.id->data.ast_id;

	// TODO:
	SymLookupResult sym_lookup = sym_tab.lookup(id.name_parts);
	if( !sym_lookup.sym || sym_lookup.sym->kind != SymKind::Type )
		return SemaError("Could not find type '" + id.name_parts.parts[0] + "'");

	return hir.create<HirTypeDeclarator>(QualifiedTy(sym_lookup.sym->data.sym_type.ty));
}

SemaResult<HirNode*>
Sema::sema_block(AstNode* ast_block)
{
	AstBlock& block = ast_block->data.ast_block;

	std::vector<HirNode*> statements;

	for( auto& stmt : block.statements )
	{
		auto stmt_result = sema_stmt(stmt);
		if( !stmt_result.ok() )
			return stmt_result;

		statements.push_back(stmt_result.unwrap());
	}

	return hir.create<HirBlock>(statements);
}

SemaResult<HirNode*>
Sema::sema_stmt(AstNode* ast_stmt)
{
	AstStmt& stmt = ast_stmt->data.ast_stmt;

	AstNode* any_stmt = stmt.stmt;

	switch( any_stmt->kind )
	{
	case NodeKind::Return:
		return sema_return(any_stmt);
	default:
		return NotImpl();
	}

	return NotImpl();
}

SemaResult<HirNode*>
Sema::sema_return(AstNode* ast_return)
{
	AstReturn& return_node = ast_return->data.ast_return;

	auto expr_result = sema_expr(return_node.expr);
	if( !expr_result.ok() )
		return expr_result;

	return hir.create<HirReturn>(expr_result.unwrap());
}

SemaResult<HirNode*>
Sema::sema_expr(AstNode* ast_expr)
{
	AstExpr& expr = ast_expr->data.ast_expr;

	AstNode* any_expr = expr.expr;

	switch( any_expr->kind )
	{
	case NodeKind::NumberLiteral:
		return sema_number_literal(any_expr);
	default:
		return NotImpl();
	}

	return NotImpl();
}

SemaResult<HirNode*>
Sema::sema_number_literal(AstNode* ast_number_literal)
{
	AstNumberLiteral& literal = ast_number_literal->data.ast_number_literal;

	return hir.create<HirNumberLiteral>(literal.value);
}