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

static std::string
to_simple(AstId& id)
{
	assert(id.kind == AstId::IdKind::Simple);

	return id.name_parts.parts[0];
}

static std::string
to_qualified(AstId& id)
{
	assert(id.kind == AstId::IdKind::Qualified);

	std::string name = id.name_parts.parts.at(0);
	for( int i = 1; i < id.name_parts.parts.size(); i++ )
		name += "::" + id.name_parts.parts.at(i);

	return name;
}

static void
union_inferences(HirNode* target, HirNode* a, HirNode* b)
{
	for( auto& inf : a->inferrences )
		target->inferrences.push_back(inf);
	for( auto& inf : b->inferrences )
		target->inferrences.push_back(inf);
}

static void
intersect_inferences(HirNode* target, HirNode* a, HirNode* b)
{
	// TODO: Better alg for this.

	for( auto& inf : a->inferrences )
	{
		auto iter_inf =
			std::find_if(b->inferrences.begin(), b->inferrences.end(), [&inf](Inferrence& elem) {
				if( elem.sym == inf.sym && QualifiedTy::equals(inf.qty, elem.qty) )
					return true;
				return false;
			});
		if( iter_inf != b->inferrences.end() )
			target->inferrences.push_back(inf);
	}
}

// @deprecated
// Due to parsing and parents ( expr ), we may have exprs pointing to exprs and so on.
// Get the innermost expr.
static AstNode*
innermost_expr(AstNode* expr)
{
	do
	{
		expr = ast_cast<AstExpr>(expr).expr;
	} while( expr->kind == NodeKind::Expr );

	return expr;
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
				std::vector<HirNode*> args{hir.create<HirId>(target, ty_sym_lu.first()), node};
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
	current_module = &statements;

	AstModule& mod = ast_cast<AstModule>(ast_module);

	for( auto& stmt : mod.statements )
	{
		switch( stmt->kind )
		{
		case NodeKind::Template:
		{
			auto stmt_result = sema_template(stmt);
			if( !stmt_result.ok() )
				return stmt_result;

			// Templates do not get added to the statements list.
		}
		break;
		default:
		{
			auto stmt_result = sema_module_stmt_any(stmt);
			if( !stmt_result.ok() )
				return stmt_result;

			statements.push_back(stmt_result.unwrap());
		}
		break;
		}
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
		return sema_union(ast_stmt);
	case NodeKind::Enum:
		return sema_enum(ast_stmt);
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

	//  We add the vars to the scope below, but we need to look up the arg types
	// here so we can include them in the func ty.
	std::vector<QualifiedTy> arg_qtys;
	for( auto& param : func_proto.parameters )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);
		AstTypeDeclarator& ast_ty = ast_cast<AstTypeDeclarator>(var_decl.type_declarator);

		SymLookupResult ty_lu = sym_tab.lookup(ast_cast<AstId>(ast_ty.id).name_parts);
		if( !ty_lu.found() )
			return SemaError("Unrecognized type.");

		arg_qtys.push_back(sym_qty(builtins, ty_lu.first()));
	}

	auto rt_type_declarator_result = type_declarator(func_proto.rt_type_declarator);
	if( !rt_type_declarator_result.ok() )
		return MoveError(rt_type_declarator_result);

	QualifiedTy rt_type_declarator = rt_type_declarator_result.unwrap().qty;

	// The function type does not need to go into the symbol table.
	// Presumably we just look up the function symbol and get the type
	// from that??
	// TODO: Enter the function in the symbol table
	// The function in the symbol table acts as an overload set
	Ty const* func_ty = types.create<TyFunc>(to_simple(id), arg_qtys, rt_type_declarator);
	Sym* sym = sym_tab.create_named<SymFunc>(to_simple(id), func_ty);

	std::vector<HirNode*> parameters;
	for( int i = 0; i < func_proto.parameters.size(); i++ )
	{
		AstNode* param = func_proto.parameters.at(i);
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);

		AstId& ast_id = ast_cast<AstId>(var_decl.id);

		QualifiedTy qty = arg_qtys.at(i);
		Sym* sym_var = sym_tab.create_named<SymVar>(to_simple(ast_id), qty);

		SymFunc& sym_func = sym->data.sym_func;
		sym_func.scope.insert(to_simple(ast_id), sym_var);
	}

	return hir.create<HirFuncProto>(sym_qty(builtins, sym), linkage, sym, parameters);
}

SemaResult<HirNode*>
Sema::sema_template(AstNode* ast_template)
{
	AstTemplate& template_nod = ast_cast<AstTemplate>(ast_template);

	// Just hang on to the typename identifiers for now.
	std::vector<AstNode*> typenames = template_nod.types;

	AstNode* template_tree = template_nod.template_tree;
	std::string name;
	SymKind kind = SymKind::Invalid;

	switch( template_nod.template_tree->kind )
	{
	case NodeKind::Func:
	{
		AstFunc& nod = ast_cast<AstFunc>(template_nod.template_tree);
		AstFuncProto& func_proto = ast_cast<AstFuncProto>(nod.proto);
		AstId& func_id = ast_cast<AstId>(func_proto.id);

		name = to_simple(func_id);
		kind = SymKind::Func;
	}
	break;
	case NodeKind::Struct:
	{
		AstStruct& nod = ast_cast<AstStruct>(template_nod.template_tree);
		AstId& id = ast_cast<AstId>(nod.id);

		name = to_simple(id);
		kind = SymKind::Type;
	}
	break;
	case NodeKind::Enum:
	{
		AstEnum& nod = ast_cast<AstEnum>(template_nod.template_tree);
		AstId& id = ast_cast<AstId>(nod.id);

		name = to_simple(id);
		kind = SymKind::Type;
	}
	break;
	case NodeKind::Union:
	{
		AstUnion& nod = ast_cast<AstUnion>(template_nod.template_tree);
		AstId& id = ast_cast<AstId>(nod.id);

		name = to_simple(id);
		kind = SymKind::Type;
	}
	break;
	default:
		return NotImpl();
	}

	sym_tab.create_named<SymTemplate>(name, kind, typenames, template_tree);
	return nullptr;
}

SemaResult<Sema::TypeDeclResult>
Sema::type_declarator(AstNode* ast_type_declarator)
{
	AstTypeDeclarator& type_declarator = ast_cast<AstTypeDeclarator>(ast_type_declarator);

	Sym* sym = nullptr;
	switch( type_declarator.id->kind )
	{
	case NodeKind::TemplateId:
	{
		SemaResult<Sym*> sym_result = lookup_or_instantiate_template(type_declarator.id);

		if( !sym_result.ok() )
			return MoveError(sym_result);

		sym = sym_result.unwrap();
	}
	break;
	case NodeKind::Id:
	{
		AstId& id = ast_cast<AstId>(type_declarator.id);

		SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
		if( !sym_lu.found() )
			return SemaError("Could not find type '" + to_qualified(id) + "'");

		sym = sym_lu.first();
	}
	break;
	default:
		return NotImpl();
	}
	if( sym->kind != SymKind::TypeAlias && sym->kind != SymKind::Type &&
		sym->kind != SymKind::EnumMember )
		return SemaError("Could not find type.");

	return TypeDeclResult{.qty = sym_qty(builtins, sym), .sym = sym};
}

SemaResult<std::map<std::string, QualifiedTy>>
Sema::decl_list(std::vector<AstNode*>& ast_decls)
{
	std::map<std::string, QualifiedTy> members;

	for( auto& ast_var_decl : ast_decls )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(ast_var_decl);

		auto type_declarator_result = type_declarator(var_decl.type_declarator);
		if( !type_declarator_result.ok() )
			return MoveError(type_declarator_result);

		AstId& member_id = ast_cast<AstId>(var_decl.id);
		std::string simple_name = to_simple(member_id);

		members.emplace(simple_name, type_declarator_result.unwrap().qty);
	}

	return members;
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
	// SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	// if( sym_lu.first_of(SymKind::Type) )
	// 	return SemaError("Redefinition");

	auto decl_list_result = decl_list(struct_nod.var_decls);
	if( !decl_list_result.ok() )
		return MoveError(decl_list_result);

	std::map<std::string, QualifiedTy> members = decl_list_result.unwrap();

	Ty const* struct_ty = types.create<TyStruct>(to_simple(id), members);
	Sym* struct_sym = sym_tab.create_named<SymType>(to_simple(id), struct_ty);

	SymType& sym = sym_cast<SymType>(struct_sym);
	sym_tab.push_scope(&sym.scope);
	int member_layout_ind = 0;
	for( auto& member : members )
		sym_tab.create_named<SymMember>(member.first, member.second, member_layout_ind++);
	sym_tab.pop_scope();

	return hir.create<HirStruct>(QualifiedTy(builtins.void_ty), struct_sym);
}

SemaResult<HirNode*>
Sema::sema_union(AstNode* ast_union)
{
	AstUnion& union_nod = ast_cast<AstUnion>(ast_union);

	AstId& id = ast_cast<AstId>(union_nod.id);
	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( sym_lu.found() )
		return SemaError("Redefinition");

	auto decl_list_result = decl_list(union_nod.var_decls);
	if( !decl_list_result.ok() )
		return MoveError(decl_list_result);

	std::map<std::string, QualifiedTy> members = decl_list_result.unwrap();

	Ty const* union_ty = types.create<TyUnion>(to_simple(id), members);
	Sym* union_sym = sym_tab.create_named<SymType>(to_simple(id), union_ty);

	SymType& sym = sym_cast<SymType>(union_sym);
	sym_tab.push_scope(&sym.scope);
	for( auto& member : members )
		sym_tab.create_named<SymMember>(member.first, member.second, 0);
	sym_tab.pop_scope();

	return hir.create<HirUnion>(QualifiedTy(builtins.void_ty), union_sym);
}

SemaResult<HirNode*>
Sema::sema_enum(AstNode* ast_enum)
{
	AstEnum& enum_nod = ast_cast<AstEnum>(ast_enum);

	AstId& id = ast_cast<AstId>(enum_nod.id);
	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( sym_lu.found() )
		return SemaError("Redefinition");

	Ty const* enum_ty = types.create<TyEnum>(to_simple(id));
	Sym* enum_sym = sym_tab.create_named<SymType>(to_simple(id), enum_ty);
	SymType& sym = sym_cast<SymType>(enum_sym);
	sym_tab.push_scope(&sym.scope);

	int member_ind = 0;
	for( auto& ast_enum_member : enum_nod.enum_member_decls )
	{
		AstEnumMember& enum_member = ast_cast<AstEnumMember>(ast_enum_member);
		AstId& member_id = ast_cast<AstId>(enum_member.id);
		std::string simple_name = to_simple(member_id);

		switch( enum_member.kind )
		{
		case AstEnumMember::MemberKind::Simple:
			sym_tab.create_named<SymEnumMember>(simple_name, member_ind);
			break;
		case AstEnumMember::MemberKind::Struct:
		{
			auto decl_list_result = decl_list(enum_member.var_decls);
			if( !decl_list_result.ok() )
				return MoveError(decl_list_result);

			Ty const* struct_ty = types.create<TyStruct>(simple_name, decl_list_result.unwrap());
			sym_tab.create_named<SymEnumMember>(simple_name, member_ind, struct_ty);
		}
		break;
		}

		member_ind += 1;
	}

	sym_tab.pop_scope();

	return hir.create<HirEnum>(QualifiedTy(builtins.void_ty), enum_sym);
}

SemaResult<HirNode*>
Sema::sema_interface(AstNode* ast_interface)
{
	return NotImpl();
	// AstInterface& interface_nod = ast_cast<AstInterface>(ast_interface);

	// // TODO: Simple name
	// AstId& id = ast_cast<AstId>(interface_nod.id);
	// SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	// if( sym_lu.found() )
	// 	return SemaError("Redefinition");

	// Ty const* union_ty = types.create<TyUnion>(to_simple(id), members);
	// Sym* union_sym = sym_tab.create_named<SymType>(to_simple(id), union_ty);

	// SymType& sym = sym_cast<SymType>(union_sym);
	// sym_tab.push_scope(&sym.scope);
	// for( auto& member : members )
	// 	sym_tab.create_named<SymMember>(member.first, member.second, 0);
	// sym_tab.pop_scope();

	// return hir.create<HirUnion>(QualifiedTy(builtins.void_ty), union_sym);
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
	case NodeKind::Switch:
		return sema_switch(any_stmt);
	case NodeKind::For:
		return sema_for(any_stmt);
	case NodeKind::While:
		return sema_while(any_stmt);
	case NodeKind::Expr:
		return sema_expr(any_stmt);
	case NodeKind::Assign:
		return sema_assign(any_stmt);
	// case NodeKind::Break:
	// 	return sema_assign(any_stmt);
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

	AstNode* sizeof_expr = innermost_expr(sizeof_nod.expr);

	HirNode* hir_expr = nullptr;
	switch( sizeof_expr->kind )
	{
	case NodeKind::Id:
	{
		// Type Name
		AstId& id = ast_cast<AstId>(sizeof_expr);

		SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
		if( !sym_lu.found() )
			return SemaError("Unrecognized id.");

		hir_expr = hir.create<HirId>(sym_qty(builtins, sym_lu.first()), sym_lu.first());
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
Sema::sema_addressof(AstNode* ast_addressof)
{
	AstAddressOf& addressof_nod = ast_cast<AstAddressOf>(ast_addressof);

	auto expr_result = sema_expr(addressof_nod.expr);
	if( !expr_result.ok() )
		return expr_result;

	// TODO: Ensure lvalueness?
	auto expr = expr_result.unwrap();

	return hir.create<HirCall>(
		QualifiedTy(builtins.bool_ty),
		HirCall::BuiltinKind::AddressOf,
		std::vector<HirNode*>{expr});
}

SemaResult<HirNode*>
Sema::sema_boolnot(AstNode* ast_boolnot)
{
	AstBoolNot& boolnot_nod = ast_cast<AstBoolNot>(ast_boolnot);

	auto expr_result = sema_expr(boolnot_nod.expr);
	if( !expr_result.ok() )
		return expr_result;

	auto expr = expr_result.unwrap();

	auto coercion_result = equal_coercion(QualifiedTy(builtins.bool_ty), expr);
	if( !coercion_result.ok() )
		return coercion_result;

	return hir.create<HirCall>(
		QualifiedTy(builtins.bool_ty),
		HirCall::BuiltinKind::BoolNot,
		std::vector<HirNode*>{coercion_result.unwrap()});
}

SemaResult<HirNode*>
Sema::sema_assign(AstNode* ast_assign)
{
	AstAssign& assign = ast_cast<AstAssign>(ast_assign);

	auto lhs_result = sema_expr(assign.lhs);
	if( !lhs_result.ok() )
		return lhs_result;

	HirNode* lhs = lhs_result.unwrap();

	auto rhs_result = sema_expr(assign.rhs);
	if( !rhs_result.ok() )
		return rhs_result;

	HirNode* rhs = lhs_result.unwrap();

	auto coercion_result = equal_coercion(lhs->qty, rhs);
	if( !coercion_result.ok() )
		return coercion_result;

	return hir.create<HirCall>(
		QualifiedTy(builtins.void_ty), BinOp::Assign, std::vector<HirNode*>({lhs, rhs}));
}

SemaResult<HirNode*>
Sema::sema_array_access(AstNode* ast_array_access)
{
	AstArrayAccess& array_access = ast_cast<AstArrayAccess>(ast_array_access);

	auto callee_result = sema_expr(array_access.callee);
	if( !callee_result.ok() )
		return callee_result;

	HirNode* callee = callee_result.unwrap();
	if( !callee->qty.is_pointer() )
		return SemaError("Array access must be a pointer or array.", array_access.callee);

	auto expr_result = sema_expr(array_access.expr);
	if( !expr_result.ok() )
		return expr_result;

	HirNode* expr = expr_result.unwrap();
	auto coercion_result = equal_coercion(QualifiedTy(builtins.i32_ty), expr);
	if( !coercion_result.ok() )
		return coercion_result;

	return hir.create<HirSubscript>(callee->qty.deref(), callee, coercion_result.unwrap());
}

SemaResult<HirNode*>
Sema::sema_member_access(AstNode* ast_member_access)
{
	AstMemberAccess& member_access = ast_cast<AstMemberAccess>(ast_member_access);

	auto callee_result = sema_expr(member_access.callee);
	if( !callee_result.ok() )
		return callee_result;

	HirNode* callee = callee_result.unwrap();
	if( member_access.kind == AstMemberAccess::AccessKind::Indirect &&
		callee->qty.indirection != 1 )
		return SemaError("-> access must be a pointer.", member_access.callee);

	if( callee->qty.ty->kind != TyKind::Struct && callee->qty.ty->kind != TyKind::Union )
		return SemaError("Cannot access member of non-struct.", member_access.callee);

	AstId& id = ast_cast<AstId>(member_access.id);

	HirMember::AccessKind kind = member_access.kind == AstMemberAccess::AccessKind::Direct
									 ? HirMember::AccessKind::Direct
									 : HirMember::AccessKind::Indirect;

	QualifiedTy member_type;
	SymLookupResult sym_lu = sym_tab.lookup(callee->qty.ty);
	if( !sym_lu.found() )
		return SemaError("???");

	SymType& type_sym = sym_cast<SymType>(sym_lu.first());
	Sym* member_sym = type_sym.scope.find(to_simple(id));
	if( !member_sym )
		return SemaError(to_simple(id) + " is not a member of struct", member_access.id);

	return hir.create<HirMember>(sym_qty(builtins, member_sym), callee, member_sym, kind);
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
		auto type_decl_result = type_declarator(var_decl.type_declarator);
		if( !type_decl_result.ok() )
			return MoveError(type_decl_result);

		TypeDeclResult tdr = type_decl_result.unwrap();
		qty = tdr.qty;
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
	Sym* sym = sym_tab.create_named<SymVar>(to_simple(id), qty);
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

		// Type inference here

		HirNode* body = nullptr;
		switch( if_nod.then_stmt->kind )
		{
		case NodeKind::DiscrimatingBlock:
		{
			sym_tab.push_scope();
			AstDiscriminatingBlock& ast_disc = ast_cast<AstDiscriminatingBlock>(if_nod.then_stmt);
			if( ast_disc.decl_list.size() > cond->inferrences.size() )
				return SemaError("Not enough type inferences to unpack.");

			for( int i = 0; i < ast_disc.decl_list.size(); i++ )
			{
				AstVarDecl& decl = ast_cast<AstVarDecl>(ast_disc.decl_list.at(i));
				Inferrence& inf = cond->inferrences.at(i);

				auto qty_result = type_declarator(decl.type_declarator);
				if( !qty_result.ok() )
					return MoveError(qty_result);

				QualifiedTy expected_qty = qty_result.unwrap().qty;
				if( !QualifiedTy::equals(expected_qty, inf.qty) )
					return SemaError("Invalid inference.");

				AstId& id = ast_cast<AstId>(decl.id);
				Sym* alias = sym_tab.create_named<SymAlias>(to_simple(id), inf.sym);
			}

			auto body_result = sema_block(ast_disc.body);
			if( !body_result.ok() )
				return body_result;

			body = body_result.unwrap();

			sym_tab.pop_scope();
		}
		break;
		case NodeKind::Stmt:
		{
			auto body_result = sema_stmt(if_nod.then_stmt);
			if( !body_result.ok() )
				return body_result;

			body = body_result.unwrap();
		}
		break;
		default:
			assert(false && "Unreachable");
			return NotImpl();
		}

		// Restore type inference

		elsifs.push_back((HirIf::CondThen){.cond = cond, .then = body});

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
Sema::sema_is(AstNode* ast_is)
{
	AstIs& is = ast_cast<AstIs>(ast_is);

	auto qty_result = type_declarator(is.type_decl);
	if( !qty_result.ok() )
		return MoveError(qty_result);
	QualifiedTy qty = qty_result.unwrap().qty;
	SymLookupResult ty_lu = sym_tab.lookup(qty.ty);
	if( !ty_lu.found() )
		return SemaError("???");
	// Must be enum type or RTTI type.
	if( ty_lu.first()->kind != SymKind::EnumMember )
		return SemaError("Must be enum type or RTTI type.");

	auto value = sym_cast<SymEnumMember>(ty_lu.first()).value;

	std::vector<HirNode*> args;
	switch( is.expr->kind )
	{
	case NodeKind::Id:
	{
		auto id_result = sema_id(is.expr);
		if( !id_result.ok() )
			return id_result;

		HirNode* hir_id = id_result.unwrap();
		Sym* sym = hir_cast<HirId>(hir_id).sym;

		// A particular id is descriminated, no lowering is needed.
		HirNode* hir_is = hir.create<HirCall>(
			QualifiedTy(builtins.bool_ty),
			HirCall::BuiltinKind::Is,
			std::vector<HirNode*>{
				hir.create<HirId>(qty, sym),									  //
				hir.create<HirNumberLiteral>(QualifiedTy(builtins.i32_ty), value) //
			});

		hir_is->inferrences.push_back(Inferrence{.qty = qty, .sym = sym});

		return hir_is;
	}
	break;
	default:
	{
		auto expr_result = sema_expr_any(is.expr);
		if( expr_result.ok() )
			return expr_result;

		HirNode* expr = expr_result.unwrap();

		// Create an anonymous symbol for reference later.
		Sym* sym = sym_tab.create<SymVar>(qty);

		// {!
		// 	let a: T;
		// 	a = expr
		// 	a
		// }
		std::vector<HirNode*> lowering({
			hir.create<HirLet>(expr->qty, sym), //
			hir.create<HirCall>(
				QualifiedTy(builtins.void_ty),
				BinOp::Assign,
				std::vector<HirNode*>{hir.create<HirId>(qty, sym), expr}), //
			hir.create<HirId>(qty, sym)									   //
		});

		HirNode* hir_lowering_block =
			hir.create<HirBlock>(qty, lowering, HirBlock::Scoping::Inherit);

		HirNode* hir_is = hir.create<HirCall>(
			QualifiedTy(builtins.bool_ty),
			HirCall::BuiltinKind::Is,
			std::vector<HirNode*>{
				hir.create<HirId>(qty, sym),									  //
				hir.create<HirNumberLiteral>(QualifiedTy(builtins.i32_ty), value) //
			});

		hir_is->inferrences.push_back(Inferrence{.qty = qty, .sym = sym});

		return hir_is;
	}
	break;
	}
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
	case NodeKind::Is:
		return sema_is(ast_expr);
	case NodeKind::SizeOf:
		return sema_sizeof(ast_expr);
	case NodeKind::MemberAccess:
		return sema_member_access(ast_expr);
	case NodeKind::ArrayAccess:
		return sema_array_access(ast_expr);
	// We can end up with next expr->expr->expr due to parens.
	case NodeKind::Expr:
		return sema_expr(ast_expr);
	default:
		return NotImpl();
	}
}

SemaResult<HirSwitch::CondThen>
Sema::sema_case(AstNode* ast_case)
{
	AstCase& case_nod = ast_cast<AstCase>(ast_case);

	AstNode* ast_cond = case_nod.cond;
	long long value;
	Sym* discrimination = nullptr;
	switch( ast_cond->kind )
	{
	case NodeKind::NumberLiteral:
	{
		AstNumberLiteral& nl = ast_cast<AstNumberLiteral>(ast_cond);
		value = nl.value;
	}
	break;
	case NodeKind::TypeDeclarator:
	{
		auto type_decl_result = type_declarator(ast_cond);
		if( !type_decl_result.ok() )
			return MoveError(type_decl_result);

		Sym* sym = type_decl_result.unwrap().sym;
		if( sym->kind != SymKind::EnumMember )
			return SemaError("Case expression must be convertable to int");

		discrimination = sym;
		value = sym_cast<SymEnumMember>(sym).value;
	}
	break;
	default:
		return SemaError("Case expression must be convertable to int");
	}

	HirNode* hir_then = nullptr;
	switch( case_nod.stmt->kind )
	{
	case NodeKind::DiscrimatingBlock:
	{
		auto disc = ast_cast<AstDiscriminatingBlock>(case_nod.stmt);
		if( disc.decl_list.size() > 1 )
			return SemaError("Too many type inferences");

		hir_then = hir.create<HirBlock>(
			QualifiedTy(builtins.void_ty), std::vector<HirNode*>(), HirBlock::Scoping::Scoped);

		HirBlock& block = hir_cast<HirBlock>(hir_then);

		sym_tab.push_scope();
		if( disc.decl_list.size() == 1 )
		{
			AstNode* ast_decl = disc.decl_list.at(0);
			AstVarDecl& decl = ast_cast<AstVarDecl>(ast_decl);
			AstId& id = ast_cast<AstId>(decl.id);

			auto qty_result = type_declarator(decl.type_declarator);
			if( !qty_result.ok() )
				return MoveError(qty_result);

			Sym* var = sym_tab.create_named<SymVar>(to_simple(id), qty_result.unwrap().qty);

			HirNode* hir_var = hir.create<HirLet>(QualifiedTy(builtins.void_ty), var);
			block.statements.push_back(hir_var);
		}

		// Body is always a block for disc.
		auto then_result = sema_stmt_any(disc.body);
		if( !then_result.ok() )
			return MoveError(then_result);
		sym_tab.pop_scope();

		block.statements.push_back(then_result.unwrap());
		break;
	}
	case NodeKind::Stmt:
	{
		auto then_result = sema_stmt(case_nod.stmt);
		if( !then_result.ok() )
			return MoveError(then_result);

		hir_then = then_result.unwrap();
		break;
	}
	default:
		return SemaError("Unexpected case statement");
	}

	return HirSwitch::CondThen{.value = value, .then = hir_then};
}

SemaResult<HirNode*>
Sema::sema_default(AstNode* ast_default)
{
	AstDefault& default_nod = ast_cast<AstDefault>(ast_default);

	return sema_stmt(default_nod.stmt);
}

SemaResult<HirNode*>
Sema::sema_id(AstNode* ast_id)
{
	AstId& id = ast_cast<AstId>(ast_id);

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.found() )
		return SemaError("Unrecognized id.");

	if( sym_lu.first()->kind == SymKind::Type )
		return SemaError("Cannot use type name as an expression.");

	return hir.create<HirId>(sym_qty(builtins, sym_lu.first()), sym_lu.first());
}

SemaResult<HirNode*>
Sema::sema_for(AstNode* ast_for)
{
	AstFor& for_nod = ast_cast<AstFor>(ast_for);

	sym_tab.push_scope();

	std::vector<HirNode*> inits;
	for( AstNode* init : for_nod.inits )
	{
		auto stmt_result = sema_stmt(init);
		if( !stmt_result.ok() )
			return stmt_result;

		inits.push_back(stmt_result.unwrap());
	}

	HirNode* cond = nullptr;
	if( for_nod.cond )
	{
		auto cond_result = sema_expr(for_nod.cond);
		if( !cond_result.ok() )
			return cond_result;
		cond = cond_result.unwrap();

		auto coercion_result = equal_coercion(QualifiedTy(builtins.bool_ty), cond);
		if( !coercion_result.ok() )
			return coercion_result;

		cond = coercion_result.unwrap();
	}

	std::vector<HirNode*> afters;
	for( AstNode* after : for_nod.afters )
	{
		auto stmt_result = sema_stmt(after);
		if( !stmt_result.ok() )
			return stmt_result;

		afters.push_back(stmt_result.unwrap());
	}

	auto body_result = sema_stmt(for_nod.body);
	if( !body_result.ok() )
		return body_result;

	HirNode* body = body_result.unwrap();
	HirNode* init =
		hir.create<HirBlock>(QualifiedTy(builtins.void_ty), inits, HirBlock::Scoping::Inherit);
	HirNode* after =
		hir.create<HirBlock>(QualifiedTy(builtins.void_ty), afters, HirBlock::Scoping::Inherit);

	sym_tab.pop_scope();

	return hir.create<HirLoop>(QualifiedTy(builtins.void_ty), init, cond, body, after);
}

SemaResult<HirNode*>
Sema::sema_while(AstNode* ast_while)
{
	AstWhile& while_nod = ast_cast<AstWhile>(ast_while);

	auto cond_result = sema_expr(while_nod.cond);
	if( !cond_result.ok() )
		return cond_result;
	HirNode* cond = cond_result.unwrap();

	auto coercion_result = equal_coercion(QualifiedTy(builtins.bool_ty), cond);
	if( !coercion_result.ok() )
		return coercion_result;

	cond = coercion_result.unwrap();

	auto body_result = sema_stmt(while_nod.body);
	if( !body_result.ok() )
		return body_result;

	HirNode* body = body_result.unwrap();
	HirNode* init = hir.create<HirBlock>(
		QualifiedTy(builtins.void_ty), std::vector<HirNode*>({}), HirBlock::Scoping::Inherit);
	HirNode* after = hir.create<HirBlock>(
		QualifiedTy(builtins.void_ty), std::vector<HirNode*>({}), HirBlock::Scoping::Inherit);

	return hir.create<HirLoop>(QualifiedTy(builtins.void_ty), init, cond, body, after);
}

SemaResult<HirNode*>
Sema::sema_switch(AstNode* ast_switch)
{
	AstSwitch& switch_nod = ast_cast<AstSwitch>(ast_switch);

	auto cond_result = sema_expr(switch_nod.cond);
	if( !cond_result.ok() )
		return cond_result;

	HirNode* cond = cond_result.unwrap();
	if( cond->qty.ty->kind != TyKind::Enum && false )
		return SemaError("Switch statement condition must be an enum or an int.");

	std::vector<HirSwitch::CondThen> branches;
	HirNode* default_block = nullptr;
	for( AstNode* branch : switch_nod.branches )
	{
		switch( branch->kind )
		{
		case NodeKind::Case:
		{
			auto case_result = sema_case(branch);
			if( !case_result.ok() )
				return MoveError(case_result);

			branches.push_back(case_result.unwrap());
			break;
		}
		case NodeKind::Default:
		{
			if( default_block )
				return SemaError("Duplicate default block");

			auto case_result = sema_default(branch);
			if( !case_result.ok() )
				return MoveError(case_result);

			default_block = case_result.unwrap();
			break;
		}
		default:
			return SemaError("Unexpected switch branch!");
		}
	}

	return hir.create<HirSwitch>(QualifiedTy(builtins.void_ty), cond, branches, default_block);
	;
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
	case NodeKind::TemplateId:
	{
		// Look up the template symbol with the given inputs.
		// Function overfload resolution does not occur since
		// the type parameters are explicitly specified, and that
		// means the symbol MUST be a template instance.
		SemaResult<Sym*> sym_result = lookup_or_instantiate_template(callee);
		if( !sym_result.ok() )
			return MoveError(sym_result);

		Sym* sym = sym_result.unwrap();
		if( sym->kind != SymKind::Func )
			return SemaError("Typename used in place of function");

		QualifiedTy qty = sym_qty(builtins, sym);
		if( !qty.is_function() )
			return SemaError("...is not a function.");

		for( int i = 0; i < args.size(); i++ )
		{
			QualifiedTy const& arg_qty = args.at(i)->qty;
			QualifiedTy const& expected_qty = ty_cast<TyFunc>(qty.ty).args_qtys.at(i);

			auto coercion_result = equal_coercion(expected_qty, args.at(i));
			if( !coercion_result.ok() )
				return coercion_result;

			args[i] = coercion_result.unwrap();
		}

		SymFunc& sym_func = sym_cast<SymFunc>(sym);
		auto& ty_func = ty_cast<TyFunc>(sym_func.ty);

		return hir.create<HirCall>(ty_func.rt_qty, sym, args);
		break;
	}
	case NodeKind::Id:
	{
		AstId& id = ast_cast<AstId>(callee);
		SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
		if( !sym_lu.found() )
			return SemaError("Unrecognized callee.");

		QualifiedTy qty = sym_qty(builtins, sym_lu.first());
		if( !qty.is_function() )
			return SemaError("...is not a function.");

		for( int i = 0; i < args.size(); i++ )
		{
			QualifiedTy const& arg_qty = args.at(i)->qty;
			QualifiedTy const& expected_qty = ty_cast<TyFunc>(qty.ty).args_qtys.at(i);

			auto coercion_result = equal_coercion(expected_qty, args.at(i));
			if( !coercion_result.ok() )
				return coercion_result;

			args[i] = coercion_result.unwrap();
		}

		auto& ty_func = ty_cast<TyFunc>(sym_cast<SymFunc>(sym_lu.first()).ty);

		return hir.create<HirCall>(ty_func.rt_qty, sym_lu.first(), args);
	}
	case NodeKind::MemberAccess:
	{
		// TODO This-call?
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
	case BinOp::Neq:
	case BinOp::Eq:
	case BinOp::Lt:
	case BinOp::Lte:
	case BinOp::Gt:
	case BinOp::Gte:
	case BinOp::And:
	case BinOp::Or:
	case BinOp::Is:
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

	if( bin_op.op == BinOp::And || bin_op.op == BinOp::Or )
		return sema_bin_op_short_circuit(ast_bin_op);
	else
		return sema_bin_op_long(ast_bin_op);
}

SemaResult<HirNode*>
Sema::sema_bin_op_long(AstNode* ast_bin_op)
{
	AstBinOp& bin_op = ast_cast<AstBinOp>(ast_bin_op);

	// TODO: If either side is a pointer type, convert to subscript inst.

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
Sema::sema_bin_op_short_circuit(AstNode* ast_bin_op)
{
	AstBinOp& bin_op = ast_cast<AstBinOp>(ast_bin_op);

	AstNode* lhs = bin_op.lhs;
	AstNode* rhs = bin_op.rhs;

	auto lhs_result = sema_expr_any(lhs);
	if( !lhs_result.ok() )
		return lhs_result;

	HirNode* hir_lhs = lhs_result.unwrap();
	if( !QualifiedTy::equals(hir_lhs->qty, QualifiedTy(builtins.bool_ty)) )
		return SemaError("Expected bool");

	auto rhs_result = sema_expr_any(rhs);
	if( !rhs_result.ok() )
		return rhs_result;

	HirNode* hir_rhs = lhs_result.unwrap();
	if( !QualifiedTy::equals(hir_rhs->qty, QualifiedTy(builtins.bool_ty)) )
		return SemaError("Expected bool");

	std::vector<HirIf::CondThen> elsifs;
	switch( bin_op.op )
	{
	case BinOp::Or:
	{
		std::vector<HirNode*> args({hir_lhs});
		HirNode* hir_sc =
			hir.create<HirCall>(QualifiedTy(builtins.bool_ty), HirCall::BuiltinKind::BoolNot, args);

		elsifs.push_back(HirIf::CondThen{.cond = hir_sc, .then = hir_rhs});

		HirNode* hir_if = hir.create<HirIf>(QualifiedTy(builtins.bool_ty), elsifs, true);
		intersect_inferences(hir_if, hir_lhs, hir_rhs);
		return hir_if;
	}
	break;
	case BinOp::And:
	{
		HirNode* hir_sc = hir_lhs;

		elsifs.push_back(HirIf::CondThen{.cond = hir_sc, .then = hir_rhs});
		HirNode* hir_if = hir.create<HirIf>(QualifiedTy(builtins.bool_ty), elsifs, true);
		// TODO: If the same symbol is inferred as different types, error.
		union_inferences(hir_if, hir_lhs, hir_rhs);
		return hir_if;
	}
	break;
	default:
		assert(false && "Unreachable");
		return SemaError("");
		break;
	}
}

SemaResult<HirNode*>
Sema::sema_number_literal(AstNode* ast_number_literal)
{
	AstNumberLiteral& literal = ast_cast<AstNumberLiteral>(ast_number_literal);
	// TODO: Check the number size and choose the type appropriately.
	return hir.create<HirNumberLiteral>(QualifiedTy(builtins.i8_ty), literal.value);
}

SemaResult<Sym*>
Sema::lookup_or_instantiate_template(AstNode* ast_template_id)
{
	AstTemplateId& template_id = ast_cast<AstTemplateId>(ast_template_id);
	AstId& id = ast_cast<AstId>(template_id.id);
	NameParts& name = id.name_parts;

	std::vector<QualifiedTy> type_params;
	for( AstNode* ast_type_decl : template_id.types )
	{
		auto type_decl_result = type_declarator(ast_type_decl);
		if( !type_decl_result.ok() )
			return MoveError(type_decl_result);

		type_params.push_back(type_decl_result.unwrap().qty);
	}

	SymLookupResult lu = sym_tab.lookup_template_instance(name, type_params);
	if( lu.found() )
		return lu.first();

	lu = sym_tab.lookup(name);
	if( !lu.found() )
		return SemaError("Unrecognized identifier used as template");

	// TODO: I guess there could be more than one template with
	// the same name and we use SFINEA
	Sym* templ = lu.first_of(SymKind::Template);
	if( !templ )
		return SemaError("Template with name not found.");

	SymTemplate& sym_templ = sym_cast<SymTemplate>(templ);

	auto save_state = sym_tab.save_state();
	sym_tab.push_scope();

	for( int i = 0; i < sym_templ.typenames.size(); i++ )
	{
		AstNode* ast_typename = sym_templ.typenames.at(i);
		QualifiedTy& qty = type_params.at(i);

		AstId& id = ast_cast<AstId>(ast_typename);

		sym_tab.create_named<SymTypeAlias>(to_simple(id), qty);
	}

	auto instantiation_result = sema_module_stmt_any(sym_templ.template_tree);
	if( !instantiation_result.ok() )
		return SemaError("Failed to instantiate template");

	HirNode* hir_stmt = instantiation_result.unwrap();
	current_module->push_back(hir_stmt);

	Sym* sym = nullptr;
	switch( hir_stmt->kind )
	{
	case HirNodeKind::Func:
		sym = hir_cast<HirFuncProto>(hir_cast<HirFunc>(hir_stmt).proto).sym;
		break;
	case HirNodeKind::Struct:
		sym = hir_cast<HirStruct>(hir_stmt).sym;
		break;
	case HirNodeKind::Union:
		sym = hir_cast<HirUnion>(hir_stmt).sym;
		break;
	case HirNodeKind::Enum:
		sym = hir_cast<HirEnum>(hir_stmt).sym;
		break;
	default:
		return NotImpl();
	}

	sym_templ.instances.push_back(SymTemplate::Instance{.sym = sym, .types = type_params});

	sym_tab.pop_scope();
	sym_tab.restore_state(save_state);

	return sym;
}