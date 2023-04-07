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
	, builtins(SymBuiltins::create_builtins(sym_tab, types))
{}

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

	return hir.create<HirModule>(QualifiedTy(builtins.void_ty), statements);
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
	auto proto = proto_result.unwrap();

	sym_tab.push_scope(&proto->data.hir_func_proto.sym->data.sym_func.scope);
	auto body_result = sema_block(func.body);
	if( !body_result.ok() )
		return body_result;
	sym_tab.pop_scope();

	return hir.create<HirFunc>(QualifiedTy(builtins.void_ty), proto, body_result.unwrap());
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

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( sym_lu.sym )
		return SemaError("Redeclaration");

	// TODO: We add the vars to the scope below, but we need to look up the arg types
	// here so we can include them in the func ty.
	std::vector<QualifiedTy> arg_qtys;
	for( auto& param : func_proto.parameters )
	{
		AstVarDecl& var_decl = param->data.ast_var_decl;
		// TODO: Simple names
		AstTypeDeclarator& ast_ty = var_decl.type_declarator->data.ast_type_declarator;

		SymLookupResult ty_lu = sym_tab.lookup(ast_ty.id->data.ast_id.name_parts);
		if( !ty_lu.sym )
			return SemaError("Unrecognized type.");

		arg_qtys.push_back(sym_qty(builtins, ty_lu.sym));
	}

	auto rt_type_declarator_result = sema_type_declarator(func_proto.rt_type_declarator);
	if( !rt_type_declarator_result.ok() )
		return rt_type_declarator_result;

	HirNode* rt_type_declarator = rt_type_declarator_result.unwrap();

	// The function type does not need to go into the symbol table.
	// Presumably we just look up the function symbol and get the type
	// from that??
	// TODO: Enter the function in the symbol table
	// The function in the symbol table acts as an overload set
	Ty const* func_ty =
		types.create<TyFunc>(id.name_parts.parts[0], arg_qtys, rt_type_declarator->qty);
	Sym* sym = sym_tab.create_named<SymFunc>(id.name_parts.parts[0], func_ty);

	std::vector<HirNode*> parameters;
	for( int i = 0; i < func_proto.parameters.size(); i++ )
	{
		AstNode* param = func_proto.parameters.at(i);
		AstVarDecl& var_decl = param->data.ast_var_decl;
		// TODO: Simple names
		AstId& ast_id = var_decl.id->data.ast_id;

		QualifiedTy qty = arg_qtys.at(i);
		Sym* sym_var = sym_tab.create_named<SymVar>(ast_id.name_parts.parts[0], qty);

		SymFunc& sym_func = sym->data.sym_func;
		sym_func.scope.insert(ast_id.name_parts.parts[0], sym_var);
	}

	return hir.create<HirFuncProto>(sym_qty(builtins, sym), linkage, sym, parameters);
}

SemaResult<HirNode*>
Sema::sema_type_declarator(AstNode* ast_type_declarator)
{
	AstTypeDeclarator& type_declarator = ast_type_declarator->data.ast_type_declarator;

	AstId& id = type_declarator.id->data.ast_id;

	// TODO:
	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.sym || sym_lu.sym->kind != SymKind::Type )
		return SemaError("Could not find type '" + id.name_parts.parts[0] + "'");

	QualifiedTy qty = sym_qty(builtins, sym_lu.sym);
	return hir.create<HirTypeDeclarator>(qty, qty);
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

	return hir.create<HirBlock>(QualifiedTy(builtins.void_ty), statements);
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

	HirNode* expr = expr_result.unwrap();

	return hir.create<HirReturn>(expr->qty, expr);
}

SemaResult<HirNode*>
Sema::sema_expr(AstNode* ast_expr)
{
	AstExpr& expr = ast_expr->data.ast_expr;

	AstNode* any_expr = expr.expr;

	return sema_expr_any(any_expr);
}

SemaResult<HirNode*>
Sema::sema_expr_any(AstNode* ast_expr)
{
	switch( ast_expr->kind )
	{
	case NodeKind::NumberLiteral:
		return sema_number_literal(ast_expr);
	case NodeKind::Id:
		return sema_id(ast_expr);
	case NodeKind::FuncCall:
		return sema_func_call(ast_expr);
	case NodeKind::BinOp:
		return sema_bin_op(ast_expr);
	default:
		return NotImpl();
	}
}

SemaResult<HirNode*>
Sema::sema_id(AstNode* ast_id)
{
	AstId& id = ast_id->data.ast_id;

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.sym )
		return SemaError("Unrecognized id.");

	return hir.create<HirId>(sym_qty(builtins, sym_lu.sym), sym_lu.sym);
}

SemaResult<HirNode*>
Sema::sema_func_call(AstNode* ast_func_call)
{
	AstFuncCall& func_call = ast_func_call->data.ast_func_call;

	std::vector<HirNode*> args;
	for( AstNode* arg : func_call.args )
	{
		auto arg_result = sema_expr(arg);
		if( !arg_result.ok() )
			return arg_result;

		args.push_back(arg_result.unwrap());
	}

	AstNode* callee = func_call.callee;
	switch( callee->kind )
	{
	case NodeKind::Id:
	{
		AstId& id = callee->data.ast_id;
		SymLookupResult sym = sym_tab.lookup(id.name_parts);
		if( !sym.sym )
			return SemaError("Unrecognized callee.");

		return hir.create<HirCall>(QualifiedTy(builtins.i32_ty), sym.sym, args);
	}
	default:
		return NotImpl();
	}
}

SemaResult<HirNode*>
Sema::sema_bin_op(AstNode* ast_bin_op)
{
	AstBinOp& bin_op = ast_bin_op->data.ast_bin_op;

	AstNode* lhs = bin_op.lhs;
	AstNode* rhs = bin_op.rhs;

	std::vector<HirNode*> args;
	for( AstNode* arg : {lhs, rhs} )
	{
		auto arg_result = sema_expr_any(arg);
		if( !arg_result.ok() )
			return arg_result;

		args.push_back(arg_result.unwrap());
	}

	return hir.create<HirCall>(QualifiedTy(builtins.i32_ty), bin_op.op, args);
}

SemaResult<HirNode*>
Sema::sema_number_literal(AstNode* ast_number_literal)
{
	AstNumberLiteral& literal = ast_number_literal->data.ast_number_literal;

	return hir.create<HirNumberLiteral>(QualifiedTy(builtins.i32_ty), literal.value);
}