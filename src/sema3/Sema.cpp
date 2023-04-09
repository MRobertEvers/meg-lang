#include "Sema.h"

#include "ast3/Ast.h"

#include <string>
#include <vector>

static SemaError
NotImpl()
{
	return SemaError("Not Implemented.");
}

template<typename T>
static SemaError
MoveError(SemaResult<T>& err)
{
	return SemaError(*err.unwrap_error().get());
}

// Checks if two types can be equal after coercing the subject type.
SemaResult<HirNode*>
Sema::equal_coercion(QualifiedTy target, HirNode* node)
{
	if( QualifiedTy::equals(target, node->qty) )
		return node;

	if( target.is_primitive() && node->qty.is_primitive() )
	{
		std::vector<Ty const*> int_tys = {
			builtins.i64_ty,
			builtins.i32_ty,
			builtins.i16_ty,
			builtins.i8_ty,
			builtins.u64_ty,
			builtins.u32_ty,
			builtins.u16_ty,
			builtins.u8_ty};
		auto target_int_ty = std::find(int_tys.begin(), int_tys.end(), target.ty);
		auto node_int_ty = std::find(int_tys.begin(), int_tys.end(), node->qty.ty);

		if( target_int_ty != int_tys.end() && node_int_ty != int_tys.end() )
		{
			auto target_ty = ty_cast<TyPrimitive>(*target_int_ty);
			auto source_ty = ty_cast<TyPrimitive>(*node_int_ty);

			if( target_ty.width > source_ty.width )
			{
				auto ty_sym_lu = sym_tab.lookup(*target_int_ty);
				std::vector<HirNode*> args{hir.create<HirId>(target, ty_sym_lu.sym), node};
				return hir.create<HirCall>(target, HirCall::BuiltinKind::Cast, args);
			}
			else
			{
				return SemaError("Implicit narrowing conversion!");
			}
		}
	}

	return SemaError("Not coercable");
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

	AstModule& mod = ast_cast<AstModule>(ast_module);

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
	case NodeKind::Struct:
		return sema_struct(ast_stmt);
	case NodeKind::Union:
	case NodeKind::Enum:
	default:
		return NotImpl();
	}
}

SemaResult<HirNode*>
Sema::sema_func(AstNode* ast_func)
{
	AstFunc& func = ast_cast<AstFunc>(ast_func);
	auto proto_result = sema_func_proto(func.proto);
	if( !proto_result.ok() )
		return proto_result;
	auto proto = proto_result.unwrap();

	HirFuncProto& hir_proto = hir_cast<HirFuncProto>(proto);
	sym_tab.push_scope(&sym_cast<SymFunc>(hir_proto.sym).scope);
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
	AstFuncProto& func_proto = ast_cast<AstFuncProto>(ast_func_proto);

	HirFuncProto::Linkage linkage = translate_linkage(func_proto.linkage);

	// TODO: Simple names only?
	AstId& id = ast_cast<AstId>(func_proto.id);

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( sym_lu.sym )
		return SemaError("Redeclaration");

	// TODO: We add the vars to the scope below, but we need to look up the arg types
	// here so we can include them in the func ty.
	std::vector<QualifiedTy> arg_qtys;
	for( auto& param : func_proto.parameters )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);
		// TODO: Simple names
		AstTypeDeclarator& ast_ty = ast_cast<AstTypeDeclarator>(var_decl.type_declarator);

		SymLookupResult ty_lu = sym_tab.lookup(ast_cast<AstId>(ast_ty.id).name_parts);
		if( !ty_lu.sym )
			return SemaError("Unrecognized type.");

		arg_qtys.push_back(sym_qty(builtins, ty_lu.sym));
	}

	auto rt_type_declarator_result = type_declarator(func_proto.rt_type_declarator);
	if( !rt_type_declarator_result.ok() )
		return MoveError(rt_type_declarator_result);

	QualifiedTy rt_type_declarator = rt_type_declarator_result.unwrap();

	// The function type does not need to go into the symbol table.
	// Presumably we just look up the function symbol and get the type
	// from that??
	// TODO: Enter the function in the symbol table
	// The function in the symbol table acts as an overload set
	Ty const* func_ty = types.create<TyFunc>(id.name_parts.parts[0], arg_qtys, rt_type_declarator);
	Sym* sym = sym_tab.create_named<SymFunc>(id.name_parts.parts[0], func_ty);

	std::vector<HirNode*> parameters;
	for( int i = 0; i < func_proto.parameters.size(); i++ )
	{
		AstNode* param = func_proto.parameters.at(i);
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);
		// TODO: Simple names
		AstId& ast_id = ast_cast<AstId>(var_decl.id);

		QualifiedTy qty = arg_qtys.at(i);
		Sym* sym_var = sym_tab.create_named<SymVar>(ast_id.name_parts.parts[0], qty);

		SymFunc& sym_func = sym->data.sym_func;
		sym_func.scope.insert(ast_id.name_parts.parts[0], sym_var);
	}

	return hir.create<HirFuncProto>(sym_qty(builtins, sym), linkage, sym, parameters);
}

SemaResult<QualifiedTy>
Sema::type_declarator(AstNode* ast_type_declarator)
{
	AstTypeDeclarator& type_declarator = ast_cast<AstTypeDeclarator>(ast_type_declarator);

	AstId& id = type_declarator.id->data.ast_id;

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.sym || sym_lu.sym->kind != SymKind::Type )
		return SemaError("Could not find type '" + id.name_parts.parts[0] + "'");

	QualifiedTy qty = sym_qty(builtins, sym_lu.sym);
	return qty;
}

SemaResult<HirNode*>
Sema::sema_block(AstNode* ast_block)
{
	AstBlock& block = ast_cast<AstBlock>(ast_block);

	std::vector<HirNode*> statements;

	for( auto& stmt : block.statements )
	{
		auto stmt_result = sema_stmt(stmt);
		if( !stmt_result.ok() )
			return stmt_result;

		statements.push_back(stmt_result.unwrap());
	}

	return hir.create<HirBlock>(
		QualifiedTy(builtins.void_ty), statements, HirBlock::Scoping::Scoped);
}

SemaResult<HirNode*>
Sema::sema_struct(AstNode* ast_struct)
{
	AstStruct& struct_nod = ast_cast<AstStruct>(ast_struct);

	// TODO: Simple name
	AstId& id = ast_cast<AstId>(struct_nod.id);
	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( sym_lu.sym )
		return SemaError("Redefinition");

	std::map<std::string, QualifiedTy> members;

	for( auto& ast_var_decl : struct_nod.var_decls )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(ast_var_decl);

		auto type_declarator_result = type_declarator(var_decl.type_declarator);
		if( !type_declarator_result.ok() )
			return MoveError(type_declarator_result);

		// TODO: Simple name
		AstId& member_id = ast_cast<AstId>(var_decl.id);

		members.emplace(member_id.name_parts.parts[0], type_declarator_result.unwrap());
	}

	Ty const* struct_ty = types.create<TyStruct>(id.name_parts.parts[0], members);
	Sym* struct_sym = sym_tab.create_named<SymType>(id.name_parts.parts[0], struct_ty);

	SymType& sym = sym_cast<SymType>(struct_sym);
	sym_tab.push_scope(&sym.scope);
	for( auto& member : members )
		sym_tab.create_named<SymMember>(member.first, member.second);
	sym_tab.pop_scope();

	return hir.create<HirStruct>(QualifiedTy(builtins.void_ty), struct_sym);
}

SemaResult<HirNode*>
Sema::sema_union(AstNode* ast_union)
{
	//
	return NotImpl();
}

SemaResult<HirNode*>
Sema::sema_stmt(AstNode* ast_stmt)
{
	AstStmt& stmt = ast_cast<AstStmt>(ast_stmt);

	AstNode* any_stmt = stmt.stmt;

	return sema_stmt_any(any_stmt);
}

SemaResult<HirNode*>
Sema::sema_stmt_any(AstNode* ast_stmt)
{
	AstNode* any_stmt = ast_stmt;

	switch( any_stmt->kind )
	{
	case NodeKind::Return:
		return sema_return(any_stmt);
	case NodeKind::Let:
		return sema_let(any_stmt);
	case NodeKind::If:
		return sema_if(any_stmt);
	case NodeKind::Block:
		return sema_block(any_stmt);
	default:
		return NotImpl();
	}

	return NotImpl();
}

SemaResult<HirNode*>
Sema::sema_return(AstNode* ast_return)
{
	AstReturn& return_node = ast_cast<AstReturn>(ast_return);

	auto expr_result = sema_expr(return_node.expr);
	if( !expr_result.ok() )
		return expr_result;

	HirNode* expr = expr_result.unwrap();

	return hir.create<HirReturn>(expr->qty, expr);
}

SemaResult<HirNode*>
Sema::sema_sizeof(AstNode* ast_sizeof)
{
	AstSizeOf& sizeof_nod = ast_cast<AstSizeOf>(ast_sizeof);

	AstNode* sizeof_expr = sizeof_nod.expr;

	HirNode* hir_expr = nullptr;
	switch( sizeof_expr->kind )
	{
	case NodeKind::Id:
	{
		// Type Name
		AstId& id = ast_cast<AstId>(sizeof_expr);

		SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
		if( !sym_lu.sym )
			return SemaError("Unrecognized id.");

		hir_expr = hir.create<HirId>(sym_qty(builtins, sym_lu.sym), sym_lu.sym);
	}
	break;
	default:
	{
		auto expr_result = sema_expr_any(sizeof_expr);
		if( !expr_result.ok() )
			return expr_result;

		hir_expr = expr_result.unwrap();
	}
	break;
	}

	std::vector<HirNode*> args{hir_expr};

	return hir.create<HirCall>(QualifiedTy(builtins.i32_ty), HirCall::BuiltinKind::SizeOf, args);
}

SemaResult<HirNode*>
Sema::sema_let(AstNode* ast_let)
{
	AstLet& let = ast_cast<AstLet>(ast_let);
	AstVarDecl& var_decl = ast_cast<AstVarDecl>(let.var_decl);

	if( !var_decl.type_declarator && !let.rhs )
		return SemaError("Let statement must specify type or an inititializing expression.");

	std::vector<HirNode*> stmts;

	QualifiedTy qty;
	if( var_decl.type_declarator )
	{
		AstTypeDeclarator& type_declarator = ast_cast<AstTypeDeclarator>(var_decl.type_declarator);
		AstId& ty_id = ast_cast<AstId>(type_declarator.id);
		SymLookupResult ty_sym_lu = sym_tab.lookup(ty_id.name_parts);
		if( !ty_sym_lu.sym )
			return SemaError("Unrecognized type.");

		qty = sym_qty(builtins, ty_sym_lu.sym);
	}

	HirNode* rhs = nullptr;
	if( let.rhs )
	{
		auto rhs_result = sema_expr(let.rhs);
		if( !rhs_result.ok() )
			return rhs_result;

		rhs = rhs_result.unwrap();

		if( qty.ty )
		{
			auto type_expr_result = equal_coercion(qty, rhs);
			if( !type_expr_result.ok() )
				return SemaError("Cannot initialize variable with bad expr type.");

			rhs = type_expr_result.unwrap();
		}
		else
		{
			qty = rhs->qty;
		}
	}

	AstId& id = ast_cast<AstId>(var_decl.id);
	// TODO: Simple name?
	Sym* sym = sym_tab.create_named<SymVar>(id.name_parts.parts[0], qty);
	stmts.push_back(hir.create<HirLet>(QualifiedTy(builtins.void_ty), sym));

	if( rhs )
	{
		std::vector<HirNode*> args{
			hir.create<HirId>(qty, sym), //
			rhs							 //
		};
		stmts.push_back(hir.create<HirCall>(
			// Assignmet is a void type
			QualifiedTy(builtins.void_ty),
			BinOp::Assign,
			args));
	}

	return hir.create<HirBlock>(QualifiedTy(builtins.void_ty), stmts, HirBlock::Scoping::Inherit);
}

SemaResult<HirNode*>
Sema::sema_if(AstNode* ast_if)
{
	std::vector<HirIf::CondThen> elsifs;
	while( ast_if && ast_if->kind == AstIf::nt )
	{
		AstIf& if_nod = ast_cast<AstIf>(ast_if);

		auto cond_result = sema_expr(if_nod.condition);
		if( !cond_result.ok() )
			return cond_result;

		HirNode* cond = cond_result.unwrap();
		auto bool_coerce_result = equal_coercion(QualifiedTy(builtins.bool_ty), cond);
		if( !bool_coerce_result.ok() )
			return bool_coerce_result;

		cond = bool_coerce_result.unwrap();

		auto body_result = sema_stmt(if_nod.then_stmt);
		if( !body_result.ok() )
			return body_result;

		elsifs.push_back((HirIf::CondThen){.cond = cond, .then = body_result.unwrap()});

		if( if_nod.else_stmt )
			ast_if = ast_cast<AstStmt>(if_nod.else_stmt).stmt;
		else
			ast_if = nullptr;
	}

	if( !ast_if )
		return hir.create<HirIf>(QualifiedTy(builtins.void_ty), elsifs, nullptr);

	// We have already unpacked the statement, so use _any here.
	auto else_result = sema_stmt_any(ast_if);
	if( !else_result.ok() )
		return else_result;

	return hir.create<HirIf>(QualifiedTy(builtins.void_ty), elsifs, else_result.unwrap());
}

SemaResult<HirNode*>
Sema::sema_expr(AstNode* ast_expr)
{
	AstExpr& expr = ast_cast<AstExpr>(ast_expr);

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
	case NodeKind::SizeOf:
		return sema_sizeof(ast_expr);
	// We can end up with next expr->expr->expr due to parens.
	case NodeKind::Expr:
		return sema_expr(ast_expr);
	default:
		return NotImpl();
	}
}

SemaResult<HirNode*>
Sema::sema_id(AstNode* ast_id)
{
	AstId& id = ast_cast<AstId>(ast_id);

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.sym )
		return SemaError("Unrecognized id.");

	if( sym_lu.sym->kind == SymKind::Type )
		return SemaError("Cannot use type name as an expression.");

	return hir.create<HirId>(sym_qty(builtins, sym_lu.sym), sym_lu.sym);
}

SemaResult<HirNode*>
Sema::sema_func_call(AstNode* ast_func_call)
{
	AstFuncCall& func_call = ast_cast<AstFuncCall>(ast_func_call);

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
		AstId& id = ast_cast<AstId>(callee);
		SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
		if( !sym_lu.sym )
			return SemaError("Unrecognized callee.");

		QualifiedTy qty = sym_qty(builtins, sym_lu.sym);
		if( !qty.is_function() )
			return SemaError("...is not a function.");

		for( int i = 0; i < args.size(); i++ )
		{
			QualifiedTy const& arg_qty = args.at(i)->qty;
			QualifiedTy const& expected_qty = ty_cast<TyFunc>(qty.ty).args_qtys.at(i);

			if( !QualifiedTy::equals(arg_qty, expected_qty) )
				return SemaError("Bad arg type.");
		}

		auto& ty_func = ty_cast<TyFunc>(sym_cast<SymFunc>(sym_lu.sym).ty);

		return hir.create<HirCall>(ty_func.rt_qty, sym_lu.sym, args);
	}
	default:
		return NotImpl();
	}
}

static QualifiedTy
bin_op_qty(SymBuiltins& builtins, BinOp op, HirNode* lhs)
{
	switch( op )
	{
	case BinOp::Lt:
	case BinOp::Lte:
	case BinOp::Gt:
	case BinOp::Gte:
	case BinOp::AndOp:
	case BinOp::OrOp:
		return QualifiedTy(builtins.bool_ty);
	case BinOp::Mul:
	case BinOp::Div:
	case BinOp::Add:
	case BinOp::Sub:
		return lhs->qty;
	default:
		return QualifiedTy(builtins.void_ty);
	}
}

SemaResult<HirNode*>
Sema::sema_bin_op(AstNode* ast_bin_op)
{
	AstBinOp& bin_op = ast_cast<AstBinOp>(ast_bin_op);

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

	// TODO: LHS might be convertable to rhs.
	auto coercion_result = equal_coercion(args[0]->qty, args[1]);
	if( !coercion_result.ok() )
		return coercion_result;

	args[1] = coercion_result.unwrap();

	return hir.create<HirCall>(bin_op_qty(builtins, bin_op.op, args[0]), bin_op.op, args);
}

SemaResult<HirNode*>
Sema::sema_number_literal(AstNode* ast_number_literal)
{
	AstNumberLiteral& literal = ast_cast<AstNumberLiteral>(ast_number_literal);
	// TODO: Check the number size and choose the type appropriately.
	return hir.create<HirNumberLiteral>(QualifiedTy(builtins.i8_ty), literal.value);
}