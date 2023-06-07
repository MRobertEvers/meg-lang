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
Sema::equal_coercion(HirNode* lhs, HirNode* node)
{
	if( QualifiedTy::equals(QualifiedTy(builtins.infer_ty), lhs->qty) )
	{
		if( lhs->kind == HirNodeKind::Id )
			sym_cast<SymVar>(hir_cast<HirId>(lhs).sym).qty = node->qty;
		lhs->qty = node->qty;
	}
	return equal_coercion(lhs->qty, node);
}

SemaResult<HirNode*>
Sema::equal_coercion(QualifiedTy target, HirNode* node)
{
	if( QualifiedTy::equals(target, node->qty) )
		return node;

	if( target.is_int() && node->qty.is_int() )
	{
		TyInt const& target_ty = ty_cast<TyInt>(target.ty);
		TyInt const& source_ty = ty_cast<TyInt>(node->qty.ty);

		if( target_ty.width > source_ty.width || source_ty.kind == TyInt::IntKind::iX )
		{
			auto ty_sym_lu = sym_tab.lookup(target.ty);
			std::vector<HirNode*> args{hir.create<HirId>(target, ty_sym_lu.first()), node};
			return hir.create<HirBuiltin>(target, HirBuiltin::BuiltinKind::IntCast, args);
		}
		else
		{
			return SemaError("Implicit narrowing conversion!");
		}
	}

	return SemaError("Not coercable");
}

Sema::Sema(Ast& ast, Hir& hir, Types& types, SymTab& sym_tab, SymBuiltins& builtins)
	: ast(ast)
	, hir(hir)
	, types(types)
	, sym_tab(sym_tab)
	, builtins(builtins)
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
	case NodeKind::FuncProto:
		return sema_func_proto(ast_stmt);
	case NodeKind::Struct:
		return sema_struct(ast_stmt);
	case NodeKind::Union:
		return sema_union(ast_stmt);
	case NodeKind::Enum:
		return sema_enum(ast_stmt);
	case NodeKind::Interface:
		return sema_interface(ast_stmt);
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

	current_function = &hir_proto;

	sym_tab.push_scope(&sym_cast<SymFunc>(hir_proto.sym).scope);
	auto body_result = sema_block(func.body);
	if( !body_result.ok() )
		return body_result;
	sym_tab.pop_scope();

	current_function = nullptr;

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

static HirFuncProto::Routine
translate_routine_kind(AstFuncProto::Routine ast_routine)
{
	switch( ast_routine )
	{
	case AstFuncProto::Routine::Subroutine:
		return HirFuncProto::Routine::Subroutine;
	case AstFuncProto::Routine::Coroutine:
		return HirFuncProto::Routine::Coroutine;
	}
}

static HirFuncProto::VarArg
translate_var_arg(AstFuncProto::VarArg ast_var_arg)
{
	switch( ast_var_arg )
	{
	case AstFuncProto::VarArg::VarArg:
		return HirFuncProto::VarArg::VarArg;
	case AstFuncProto::VarArg::None:
		return HirFuncProto::VarArg::None;
	}
}

SemaResult<HirNode*>
Sema::sema_func_proto(AstNode* ast_func_proto)
{
	AstFuncProto& func_proto = ast_cast<AstFuncProto>(ast_func_proto);

	HirFuncProto::Linkage linkage = translate_linkage(func_proto.linkage);
	HirFuncProto::Routine routine_kind = translate_routine_kind(func_proto.routine);
	HirFuncProto::VarArg var_arg = translate_var_arg(func_proto.var_arg);

	AstId& id = ast_cast<AstId>(func_proto.id);

	//  We add the vars to the scope below, but we need to look up the arg types
	// here so we can include them in the func ty.
	std::vector<QualifiedTy> arg_qtys;
	for( auto& param : func_proto.parameters )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);
		AstTypeDeclarator& ast_ty = ast_cast<AstTypeDeclarator>(var_decl.type_declarator);
		auto type_declarator_result = type_declarator(var_decl.type_declarator);

		arg_qtys.push_back(type_declarator_result.unwrap().qty);
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
	std::string name = to_simple(id);
	Ty const* func_ty = types.create<TyFunc>(
		name, arg_qtys, rt_type_declarator, func_proto.var_arg == AstFuncProto::VarArg::VarArg);
	Sym* sym = sym_tab.create_named<SymFunc>(name, func_ty);
	if( name == "main" )
		linkage = HirFuncProto::Linkage::Extern;

	SymFunc& sym_func = sym->data.sym_func;
	std::vector<HirNode*> parameters;

	// Add all the function parameters to the function scope.
	sym_tab.push_scope(&sym_func.scope);
	for( int i = 0; i < func_proto.parameters.size(); i++ )
	{
		AstNode* param = func_proto.parameters.at(i);
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(param);

		AstId& ast_id = ast_cast<AstId>(var_decl.id);

		QualifiedTy qty = arg_qtys.at(i);
		Sym* sym_var = sym_tab.create_named<SymVar>(to_simple(ast_id), qty);

		parameters.push_back(hir.create<HirId>(qty, sym_var));
	}
	sym_tab.pop_scope();

	return hir.create<HirFuncProto>(
		sym_qty(builtins, sym), linkage, routine_kind, sym, parameters, var_arg);
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
	case NodeKind::Interface:
	{
		AstInterface& nod = ast_cast<AstInterface>(template_nod.template_tree);
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

void
inherit_scope(SymTab& sym_tab, SymScope& base, SymScope& in)
{
	for( auto& [name, sym] : in )
	{
		// Interfaces create function symbols for their function members so
		// we can just clone those.
		sym_tab.clone_symbol_to(&base, name, sym);
	}
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

	sym = sym_unalias(sym);

	if( sym->kind != SymKind::Type && sym->kind != SymKind::EnumMember )
		return SemaError("Could not find type.");

	QualifiedTy qty = sym_qty(builtins, sym);
	if( type_declarator.impl_kind == AstTypeDeclarator::ImplKind::Impl )
	{
		if( !qty.is_interface() )
			return SemaError("Cannot 'impl' a non-interface.");

		qty.impl = QualifiedTy::ImplKind::Impl;

		std::string name = "impl " + ty_cast<TyInterface>(qty.ty).name;

		// 'impl's create anonymous types of interfaces.
		Ty const* impl_ty = types.create<TyStruct>(name, std::vector<Ty const*>{qty.ty});

		// TODO: "Inherit scope!"
		SymType& interface_sym = sym_cast<SymType>(sym);
		sym = sym_tab.create<SymType>(impl_ty);
		SymType& anonymous_sym = sym_cast<SymType>(sym);
		inherit_scope(sym_tab, anonymous_sym.scope, interface_sym.scope);

		qty = QualifiedTy(impl_ty);

		// Do not create a struct hir_node here, the codegen will use the rt_ty sym
		// to generate the correct struct.
		// HirNode* hir_impl_generator_struct = hir.create<HirStruct>(qty, sym);
		// current_module->push_back(hir_impl_generator_struct);
	}

	qty.indirection = type_declarator.indirection;

	return TypeDeclResult{.qty = qty, .sym = sym};
}

SemaResult<std::map<std::string, Member>>
Sema::decl_list(std::vector<AstNode*>& ast_decls)
{
	std::map<std::string, Member> members;

	int ind = 0;
	for( auto& ast_var_decl : ast_decls )
	{
		AstVarDecl& var_decl = ast_cast<AstVarDecl>(ast_var_decl);

		auto type_declarator_result = type_declarator(var_decl.type_declarator);
		if( !type_declarator_result.ok() )
			return MoveError(type_declarator_result);

		AstId& member_id = ast_cast<AstId>(var_decl.id);
		std::string simple_name = to_simple(member_id);

		members.emplace(simple_name, Member(ind++, type_declarator_result.unwrap().qty));
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

	AstId& id = ast_cast<AstId>(struct_nod.id);
	// SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	// if( sym_lu.first_of(SymKind::Type) )
	// 	return SemaError("Redefinition");

	auto decl_list_result = decl_list(struct_nod.var_decls);
	if( !decl_list_result.ok() )
		return MoveError(decl_list_result);

	std::map<std::string, Member> members = decl_list_result.unwrap();

	Ty const* struct_ty = types.create<TyStruct>(to_simple(id), members);
	Sym* struct_sym = sym_tab.create_named<SymType>(to_simple(id), struct_ty);

	SymType& sym = sym_cast<SymType>(struct_sym);
	sym_tab.push_scope(&sym.scope);
	int member_layout_ind = 0;
	for( auto& member : members )
		sym_tab.create_named<SymMember>(member.first, member.second);
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

	std::map<std::string, Member> members = decl_list_result.unwrap();

	Ty const* union_ty = types.create<TyUnion>(to_simple(id), members);
	Sym* union_sym = sym_tab.create_named<SymType>(to_simple(id), union_ty);

	SymType& sym = sym_cast<SymType>(union_sym);
	sym_tab.push_scope(&sym.scope);
	for( auto& member : members )
		sym_tab.create_named<SymMember>(member.first, member.second);
	sym_tab.pop_scope();

	return hir.create<HirUnion>(QualifiedTy(builtins.void_ty), union_sym);
}

SemaResult<HirNode*>
Sema::sema_enum(AstNode* ast_enum)
{
	AstEnum& enum_nod = ast_cast<AstEnum>(ast_enum);

	AstId& id = ast_cast<AstId>(enum_nod.id);
	// SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	// if( sym_lu.found() )
	// 	return SemaError("Redefinition");

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

struct ProtoNameResult
{
	std::string name;
	QualifiedTy qty;
};
static ProtoNameResult
proto_name(HirNode* hir_fn_proto)
{
	HirFuncProto& fn_proto = hir_cast<HirFuncProto>(hir_fn_proto);
	Sym* sym = fn_proto.sym;
	SymFunc& func = sym_cast<SymFunc>(sym);
	TyFunc const& func_ty = ty_cast<TyFunc>(func.ty);

	return ProtoNameResult{.name = func_ty.name, .qty = QualifiedTy(func.ty)};
}

SemaResult<HirNode*>
Sema::sema_interface(AstNode* ast_interface)
{
	AstInterface& interface_nod = ast_cast<AstInterface>(ast_interface);

	AstId& id = ast_cast<AstId>(interface_nod.id);
	// SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	// if( sym_lu.found() )
	// 	return SemaError("Redefinition");

	// This is unusual, create the sym first, later we will fill in the type.
	// I didn't want to break the function proto sema into multiple steps,
	// (1. Proto Ty, 2. Proto Symbol)
	// Since sema_func_proto populates symbols in scope,
	// we need the scope before we call that function.
	std::map<std::string, Member> members;
	Ty const* interface_ty = types.create<TyInterface>(to_simple(id), members);
	Sym* interface_sym = sym_tab.create_named<SymType>(to_simple(id), interface_ty);

	SymType& sym = sym_cast<SymType>(interface_sym);
	sym_tab.push_scope(&sym.scope);

	int ind = 0;
	for( AstNode* ast_decl : interface_nod.members )
	{
		auto member_result = sema_interface_member(ast_decl);
		if( !member_result.ok() )
			return member_result;

		if( ast_decl->kind == NodeKind::FuncProto )
		{
			HirNode* hir_fn = member_result.unwrap();
			ProtoNameResult pnr = proto_name(hir_fn);
			members.emplace(pnr.name, Member(ind++, pnr.qty));
		}
	}

	const_cast<Ty*>(interface_ty)->data.ty_interface.members = members;

	sym.ty = interface_ty;

	sym_tab.pop_scope();

	return hir.create<HirInterface>(QualifiedTy(builtins.void_ty), interface_sym);
}

SemaResult<HirNode*>
Sema::sema_interface_member(AstNode* ast_node)
{
	switch( ast_node->kind )
	{
	case NodeKind::Using:
		return sema_using(ast_node);
	default:
		return sema_func_proto(ast_node);
	}
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

	return hir.create<HirBuiltin>(
		QualifiedTy(builtins.i32_ty), HirBuiltin::BuiltinKind::SizeOf, args);
}

SemaResult<HirNode*>
Sema::sema_addressof(AstNode* ast_addressof)
{
	AstAddressOf& addressof_nod = ast_cast<AstAddressOf>(ast_addressof);

	auto expr_result = sema_expr_any(addressof_nod.expr);
	if( !expr_result.ok() )
		return expr_result;

	// TODO: Ensure lvalueness?
	HirNode* expr = expr_result.unwrap();

	return hir.create<HirBuiltin>(
		expr->qty.pointer_to(), HirBuiltin::BuiltinKind::AddressOf, std::vector<HirNode*>{expr});
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

	return hir.create<HirBuiltin>(
		QualifiedTy(builtins.bool_ty),
		HirBuiltin::BuiltinKind::BoolNot,
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

	return sema_assign(lhs, assign.rhs);
}

SemaResult<HirNode*>
Sema::sema_assign(HirNode* lhs, AstNode* ast_rhs)
{
	AstNode* expr_any = ast_cast<AstExpr>(ast_rhs).expr;
	if( expr_any->kind == NodeKind::FuncCall )
		return sema_func_call(expr_any, lhs);
	else if( expr_any->kind == NodeKind::Initializer )
		return sema_initializer(expr_any, lhs);

	auto rhs_result = sema_expr_any(expr_any);
	if( !rhs_result.ok() )
		return rhs_result;

	HirNode* rhs = rhs_result.unwrap();

	auto coercion_result = equal_coercion(lhs, rhs);
	if( !coercion_result.ok() )
		return coercion_result;

	if( expr_any->kind == NodeKind::Initializer || expr_any->kind == NodeKind::FuncCall )
		return coercion_result;
	else
		return hir.create<HirBinOp>(
			QualifiedTy(builtins.void_ty), BinOp::Assign, lhs, coercion_result.unwrap());
}

SemaResult<HirNode*>
Sema::sema_array_access(AstNode* ast_array_access)
{
	AstArrayAccess& array_access = ast_cast<AstArrayAccess>(ast_array_access);

	auto callee_result = sema_expr_any(array_access.callee);
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

	auto callee_result = sema_expr_any(member_access.callee);
	if( !callee_result.ok() )
		return callee_result;

	HirNode* callee = callee_result.unwrap();
	if( member_access.kind == AstMemberAccess::AccessKind::Indirect &&
		callee->qty.indirection != 1 )
		return SemaError("-> access must be a pointer.", member_access.callee);
	else if(
		member_access.kind == AstMemberAccess::AccessKind::Direct && callee->qty.indirection != 0 )
		return SemaError(". access must be a reference.", member_access.callee);

	if( callee->qty.ty->kind != TyKind::Struct && callee->qty.ty->kind != TyKind::Union )
		return SemaError("Cannot access member of non-struct.", member_access.callee);

	AstId& id = ast_cast<AstId>(member_access.id);

	SymLookupResult sym_lu = sym_tab.lookup(callee->qty.ty);
	if( !sym_lu.found() )
		return SemaError("???");

	SymType& type_sym = sym_cast<SymType>(sym_lu.first());
	Sym* member_sym = type_sym.scope.find(to_simple(id));
	if( !member_sym )
		return SemaError(to_simple(id) + " is not a member of struct", member_access.id);

	if( member_access.kind == AstMemberAccess::AccessKind::Indirect )
	{
		callee = hir.create<HirBuiltin>(
			callee->qty.deref(), HirBuiltin::BuiltinKind::Deref, std::vector<HirNode*>({callee}));
	}

	return hir.create<HirMember>(sym_qty(builtins, member_sym), callee, member_sym);
}

SemaResult<HirNode*>
Sema::sema_let(AstNode* ast_let)
{
	AstLet& let = ast_cast<AstLet>(ast_let);
	AstVarDecl& var_decl = ast_cast<AstVarDecl>(let.var_decl);

	if( !var_decl.type_declarator && !let.rhs )
		return SemaError("Let statement must specify type or an inititializing expression.");

	std::vector<HirNode*> stmts;

	QualifiedTy qty = QualifiedTy(builtins.infer_ty);
	if( var_decl.type_declarator )
	{
		auto type_decl_result = type_declarator(var_decl.type_declarator);
		if( !type_decl_result.ok() )
			return MoveError(type_decl_result);

		TypeDeclResult tdr = type_decl_result.unwrap();
		qty = tdr.qty;
	}

	AstId& id = ast_cast<AstId>(var_decl.id);
	Sym* sym = sym_tab.create_named<SymVar>(to_simple(id), qty);

	HirNode* hir_let = hir.create<HirLet>(QualifiedTy(builtins.void_ty), sym);

	current_function->locals.push_back(hir_let);
	stmts.push_back(hir_let);

	// HirId is void?? Should be rhs
	HirNode* lhs = hir.create<HirId>(qty, sym);

	HirNode* rhs = nullptr;
	if( let.rhs )
	{
		auto assign_result = sema_assign(lhs, let.rhs);
		if( !assign_result.ok() )
			return assign_result;

		HirNode* rhs = assign_result.unwrap();

		stmts.push_back(rhs);
	}

	// AstId& id = ast_cast<AstId>(var_decl.id);
	// Sym* sym = sym_tab.create_named<SymVar>(to_simple(id), qty);
	// stmts.push_back(hir.create<HirLet>(QualifiedTy(builtins.void_ty), sym));

	// if( rhs )
	// {
	// 	std::vector<HirNode*> args{
	// 		hir.create<HirId>(qty, sym), //
	// 		rhs							 //
	// 	};
	// 	stmts.push_back(hir.create<HirCall>(
	// 		// Assignmet is a void type
	// 		QualifiedTy(builtins.void_ty),
	// 		BinOp::Assign,
	// 		args));
	// }

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
Sema::sema_initializer(AstNode* ast_initializer)
{
	return sema_initializer(ast_initializer, nullptr);
}

SemaResult<HirNode*>
Sema::sema_initializer(AstNode* ast_initializer, HirNode* hir_self)
{
	AstInitializer& init = ast_cast<AstInitializer>(ast_initializer);

	AstId& id = ast_cast<AstId>(init.id);

	SymLookupResult sym_lu = sym_tab.lookup(id.name_parts);
	if( !sym_lu.found() || !sym_lu.first_of(SymKind::Type) )
		return SemaError("Not found.");
	Sym* sym = sym_lu.first_of(SymKind::Type);
	SymType& ty_sym = sym_cast<SymType>(sym);

	std::vector<HirNode*> designators;
	if( !hir_self )
	{
		Sym* var = sym_tab.create<SymVar>(QualifiedTy(ty_sym.ty));
		HirNode* hir_decl = hir.create<HirLet>(QualifiedTy(builtins.void_ty), var);
		hir_self = hir.create<HirId>(QualifiedTy(builtins.void_ty), var);

		designators.push_back(hir_decl);
	}

	for( auto& ast_designator : init.designators )
	{
		AstDesignator& designator = ast_cast<AstDesignator>(ast_designator);
		AstId& designator_id = ast_cast<AstId>(designator.id);

		Sym* member_sym = ty_sym.scope.find(to_simple(designator_id));
		if( !member_sym )
			return SemaError(to_simple(designator_id) + " is not a member of type");

		QualifiedTy needed_qty = sym_qty(builtins, member_sym);
		HirNode* member = hir.create<HirMember>(QualifiedTy(needed_qty), hir_self, member_sym);

		auto assign_result = sema_assign(member, designator.expr);
		if( !assign_result.ok() )
			return assign_result;

		designators.push_back(assign_result.unwrap());
	}

	designators.push_back(hir_self);

	return hir.create<HirBlock>(sym_qty(builtins, sym), designators, HirBlock::Scoping::Inherit);
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
		HirNode* hir_is = hir.create<HirBuiltin>(
			QualifiedTy(builtins.bool_ty),
			HirBuiltin::BuiltinKind::Is,
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

		// TODO: Not sure about this, assignment might need the same treatment
		// as sema_assign, where we pass values etc. func_call vs initializer.
		std::vector<HirNode*> lowering({
			hir.create<HirLet>(expr->qty, sym), //
			hir.create<HirBinOp>(
				QualifiedTy(builtins.void_ty),
				BinOp::Assign,
				hir.create<HirId>(qty, sym),
				expr),					//
			hir.create<HirId>(qty, sym) //
		});

		HirNode* hir_lowering_block =
			hir.create<HirBlock>(qty, lowering, HirBlock::Scoping::Inherit);

		HirNode* hir_is = hir.create<HirBuiltin>(
			QualifiedTy(builtins.bool_ty),
			HirBuiltin::BuiltinKind::Is,
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
Sema::sema_using(AstNode* ast_using)
{
	AstUsing& using_nod = ast_cast<AstUsing>(ast_using);

	AstId& lhs = ast_cast<AstId>(using_nod.id_lhs);
	AstId& rhs = ast_cast<AstId>(using_nod.id_rhs);
	SymLookupResult sym_lu = sym_tab.lookup(rhs.name_parts);
	if( !sym_lu.found() )
		return SemaError("Unrecognized name");

	sym_tab.create_named<SymAlias>(to_simple(lhs), sym_lu.first());

	return nullptr;
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
	case NodeKind::StringLiteral:
		return sema_string_literal(ast_expr);
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
	case NodeKind::Yield:
		return sema_yield(ast_expr);
	case NodeKind::Initializer:
		return sema_initializer(ast_expr);
	// We can end up with next expr->expr->expr due to parens.
	case NodeKind::Expr:
		return sema_expr(ast_expr);
	case NodeKind::AddressOf:
		return sema_addressof(ast_expr);
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

QualifiedTy
gen_send_qty(SymBuiltins& builtins, Sym* sym)
{
	SymType& gen = sym_cast<SymType>(sym);
	Sym* send = gen.scope.find("Send");
	assert(send);

	return sym_qty(builtins, send);
}

QualifiedTy
gen_iter_qty(SymBuiltins& builtins, Sym* sym)
{
	SymType& gen = sym_cast<SymType>(sym);
	Sym* send = gen.scope.find("Iter");
	assert(send);

	return sym_qty(builtins, send);
}

QualifiedTy
gen_ret_qty(SymBuiltins& builtins, Sym* sym)
{
	SymType& gen = sym_cast<SymType>(sym);
	Sym* send = gen.scope.find("Ret");
	assert(send);

	return sym_qty(builtins, send);
}

SemaResult<HirNode*>
Sema::sema_yield(AstNode* ast_yield)
{
	assert(current_function);
	if( current_function->kind != HirFuncProto::Routine::Coroutine )
		return SemaError("Yield statement can only be used in 'async' functions.");

	AstYield& yield_nod = ast_cast<AstYield>(ast_yield);

	auto expr_result = sema_expr_any(yield_nod.expr);
	if( !expr_result.ok() )
		return expr_result;

	SymFunc& func = sym_cast<SymFunc>(current_function->sym);
	SymLookupResult yield_ty_sym = sym_tab.lookup(ty_cast<TyFunc>(func.ty).rt_qty.ty);
	if( !yield_ty_sym.found() )
		return SemaError("??");

	QualifiedTy yield_qty = gen_iter_qty(builtins, yield_ty_sym.first());
	HirNode* expr = expr_result.unwrap();
	auto coercion_result = equal_coercion(yield_qty, expr);
	if( !coercion_result.ok() )
		return coercion_result;

	QualifiedTy send_qty = gen_send_qty(builtins, yield_ty_sym.first());
	return hir.create<HirYield>(send_qty, coercion_result.unwrap());
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
		// This type of parsing works for the switch statement
		// because 'case' and 'default' must appear as top level definitions.
		//
		// e.g.
		// switch (e) {
		//   case Blurn:
		//   {
		//		case Blurn2: <--- not allowed
		//   }
		//
		// For parsing 'break', which is not used in switch statements
		// We have to keep track of the current break target.
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
}

SemaResult<HirNode*>
Sema::sema_func_call(AstNode* ast_func_call, HirNode* hir_lhs)
{
	auto func_call_resullt = sema_func_call(ast_func_call);
	if( !func_call_resullt.ok() )
		return func_call_resullt;

	HirNode* hir_call = func_call_resullt.unwrap();

	auto coercion_result = equal_coercion(hir_lhs, hir_call);
	if( !coercion_result.ok() )
		return coercion_result;

	HirNode* assignment = coercion_result.unwrap();

	return hir.create<HirConstruct>(assignment->qty, hir_lhs, assignment);
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

		return hir.create<HirFuncCall>(ty_func.rt_qty, sym, args);
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

		TyFunc const& func_ty = ty_cast<TyFunc>(qty.ty);
		if( args.size() < func_ty.args_qtys.size() )
			return SemaError("Not enough args");

		for( int i = 0; i < args.size(); i++ )
		{
			QualifiedTy const& arg_qty = args.at(i)->qty;
			if( i == func_ty.args_qtys.size() )
			{
				if( func_ty.is_var_arg )
					break;
				else
					return SemaError("Too many arguments");
			}

			QualifiedTy const& expected_qty = func_ty.args_qtys.at(i);

			auto coercion_result = equal_coercion(expected_qty, args.at(i));
			if( !coercion_result.ok() )
				return coercion_result;

			args[i] = coercion_result.unwrap();
		}

		auto& ty_func = ty_cast<TyFunc>(sym_cast<SymFunc>(sym_lu.first()).ty);

		return hir.create<HirFuncCall>(ty_func.rt_qty, sym_lu.first(), args);
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
bin_op_qty(SymBuiltins& builtins, BinOp op, QualifiedTy input_tys)
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
		return input_tys;
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

	bool some_ptr = std::any_of(
		args.begin(), args.end(), [&](HirNode* hir) { return hir->qty.is_subscriptable(); });
	if( (bin_op.op == BinOp::Add || bin_op.op == BinOp::Sub) && some_ptr )
		return ptr_arithmetic(bin_op.op, args);

	// We are performing operations on numbers. If the types are ints,
	// then perform integer promotion
	// Promotion rules:
	// 1. If both operands are smaller than i32, convert to i32.
	// 2. Otherwise promote to the larger size.
	bool all_ints = std::all_of(args.begin(), args.end(), [&](HirNode* hir) {
		return hir->qty.is_int(); //
	});
	if( all_ints )
		return int_arithmetic(bin_op.op, args);

	return NotImpl();
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

	HirNode* hir_rhs = rhs_result.unwrap();
	if( !QualifiedTy::equals(hir_rhs->qty, QualifiedTy(builtins.bool_ty)) )
		return SemaError("Expected bool");

	std::vector<HirIf::CondThen> elsifs;
	switch( bin_op.op )
	{
	case BinOp::Or:
	{
		std::vector<HirNode*> args({hir_lhs});
		HirNode* hir_sc = hir.create<HirBuiltin>(
			QualifiedTy(builtins.bool_ty), HirBuiltin::BuiltinKind::BoolNot, args);

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
	return hir.create<HirNumberLiteral>(QualifiedTy(builtins.ix_ty), literal.value);
}

SemaResult<HirNode*>
Sema::sema_string_literal(AstNode* ast_string_literal)
{
	AstStringLiteral& literal = ast_cast<AstStringLiteral>(ast_string_literal);
	// TODO: Check the number size and choose the type appropriately.
	return hir.create<HirStringLiteral>(
		QualifiedTy(builtins.i8_ty, 1), literal.value.substr(1, literal.value.size() - 2));
}

static std::vector<QualifiedTy>
types_from_decls(std::vector<Sema::TypeDeclResult>& type_param_decls)
{
	std::vector<QualifiedTy> l;
	for( auto& tpd : type_param_decls )
		l.push_back(tpd.qty);
	return l;
}

SemaResult<Sym*>
Sema::lookup_or_instantiate_template(AstNode* ast_template_id)
{
	AstTemplateId& template_id = ast_cast<AstTemplateId>(ast_template_id);
	AstId& id = ast_cast<AstId>(template_id.id);
	NameParts& name = id.name_parts;

	std::vector<Sema::TypeDeclResult> type_param_decls;
	for( AstNode* ast_type_decl : template_id.types )
	{
		auto type_decl_result = type_declarator(ast_type_decl);
		if( !type_decl_result.ok() )
			return MoveError(type_decl_result);

		type_param_decls.push_back(type_decl_result.unwrap());
	}

	std::vector<QualifiedTy> type_params = types_from_decls(type_param_decls);

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
	// TODO: Push the namespace that the symbol belongs.
	sym_tab.push_scope();

	// Match the symbol of the typename to the concrete type argument symbol.
	// E.g. interface Blah<typename Har, typename Bar> {
	//
	// }
	// =>
	// usage_fn() { Blah<i32, i64>(); }
	//
	// map "Har" => "i32" and "Bar" => "i64"
	for( int i = 0; i < sym_templ.typenames.size(); i++ )
	{
		AstNode* ast_typename = sym_templ.typenames.at(i);
		Sym* typename_sym = type_param_decls.at(i).sym;

		AstId& typename_id = ast_cast<AstId>(ast_typename);

		sym_tab.create_named<SymAlias>(to_simple(typename_id), typename_sym);
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
	case HirNodeKind::Interface:
		sym = hir_cast<HirInterface>(hir_stmt).sym;
		break;
	default:
		return NotImpl();
	}

	sym_templ.instances.push_back(SymTemplate::Instance{.sym = sym, .types = type_params});

	sym_tab.pop_scope();
	sym_tab.restore_state(save_state);

	return sym;
}

SemaResult<HirNode*>
Sema::ptr_arithmetic(BinOp op, std::vector<HirNode*> args)
{
	bool one_of = std::all_of(
		args.begin(), args.end(), [&](HirNode* hir) { return hir->qty.is_subscriptable(); });
	if( one_of )
		return SemaError("Cannot perform arithmetic between two pointers.");

	HirNode* ptr = args[0]->qty.is_subscriptable() ? args[0] : args[1];
	HirNode* number = args[0]->qty.is_subscriptable() ? args[1] : args[0];

	auto number_coercion = equal_coercion(QualifiedTy(builtins.i32_ty), number);
	if( !number_coercion.ok() )
		return number_coercion;

	number = number_coercion.unwrap();

	// ptr-qty is dummy here.
	HirNode* ptr_deref = hir.create<HirSubscript>(ptr->qty, ptr, number);

	return hir.create<HirBuiltin>(
		ptr->qty, HirBuiltin::BuiltinKind::AddressOf, std::vector<HirNode*>({ptr_deref}));
}

HirNode*
Sema::int_cast(HirNode* node, int width)
{
	QualifiedTy qty = int_qty(builtins, width, true);
	Sym* sym = sym_tab.lookup(qty.ty).first();

	std::vector<HirNode*> args{hir.create<HirId>(int_qty(builtins, width, true), sym), node};
	return hir.create<HirBuiltin>(
		int_qty(builtins, width, true), HirBuiltin::BuiltinKind::IntCast, args);
}

SemaResult<HirNode*>
Sema::int_arithmetic(BinOp op, std::vector<HirNode*> args)
{
	bool all_ints = std::all_of(args.begin(), args.end(), [&](HirNode* hir) {
		return hir->qty.is_int(); //
	});
	if( !all_ints || args.size() != 2 )
		return SemaError("Integer arithmetic can only be performed on ints");

	HirNode* lhs = args[0];
	HirNode* rhs = args[1];
	QualifiedTy lhs_qty = lhs->qty;
	QualifiedTy rhs_qty = rhs->qty;
	TyInt const& lhs_ty = ty_cast<TyInt>(lhs_qty.ty);
	TyInt const& rhs_ty = ty_cast<TyInt>(rhs_qty.ty);
	QualifiedTy qty;

	if( lhs_ty.width <= 32 && rhs_ty.width <= 32 )
	{
		if( lhs_ty.width <= 32 )
			lhs = int_cast(lhs, 32);
		if( rhs_ty.width <= 32 )
			rhs = int_cast(rhs, 32);

		qty = int_qty(builtins, 32, true);
	}
	else
	{
		int max_width = lhs_ty.width > rhs_ty.width ? lhs_ty.width : rhs_ty.width;
		if( lhs_ty.width <= max_width )
			lhs = int_cast(lhs, max_width);
		if( rhs_ty.width <= max_width )
			rhs = int_cast(rhs, max_width);

		qty = int_qty(builtins, max_width, true);
	}

	qty = bin_op_qty(builtins, op, qty);

	return hir.create<HirBinOp>(qty, op, lhs, rhs);
}