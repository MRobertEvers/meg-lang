#include "SymBuiltins.h"

#include "QualifiedTy.h"

SymBuiltins
SymBuiltins::create_builtins(SymTab& sym_tab, Types& types)
{
	SymBuiltins builtins;
	Ty const* ty = nullptr;
	ty = types.create<TyPrimitive>("i64", 64);
	sym_tab.create_named<SymType>("i64", ty);
	builtins.i64_ty = ty;
	ty = types.create<TyPrimitive>("i32", 32);
	sym_tab.create_named<SymType>("i32", ty);
	builtins.i32_ty = ty;
	ty = types.create<TyPrimitive>("i16", 16);
	sym_tab.create_named<SymType>("i16", ty);
	builtins.i16_ty = ty;
	ty = types.create<TyPrimitive>("i8", 8);
	sym_tab.create_named<SymType>("i8", ty);
	builtins.i8_ty = ty;
	ty = types.create<TyPrimitive>("u64", 64);
	sym_tab.create_named<SymType>("u64", ty);
	builtins.u64_ty = ty;
	ty = types.create<TyPrimitive>("u32", 32);
	sym_tab.create_named<SymType>("u32", ty);
	builtins.u32_ty = ty;
	ty = types.create<TyPrimitive>("u16", 16);
	sym_tab.create_named<SymType>("u16", ty);
	builtins.u16_ty = ty;
	ty = types.create<TyPrimitive>("u8", 8);
	sym_tab.create_named<SymType>("u8", ty);
	builtins.u8_ty = ty;

	ty = types.create<TyPrimitive>("bool", 1);
	sym_tab.create_named<SymType>("bool", ty);
	builtins.bool_ty = ty;

	ty = types.create<TyPrimitive>("void", 0);
	sym_tab.create_named<SymType>("void", ty);
	builtins.void_ty = ty;

	return builtins;
}

QualifiedTy
sym_qty(SymBuiltins const& builtins, Sym* sym)
{
	switch( sym->kind )
	{
	case SymKind::Var:
		return sym->data.sym_var.qty;
	case SymKind::Func:
		return QualifiedTy(sym->data.sym_func.ty);
	case SymKind::Type:
		return QualifiedTy(sym->data.sym_type.ty);
	case SymKind::Member:
		return sym->data.sym_member.qty;
	case SymKind::Template:
		return QualifiedTy(builtins.void_ty);
	case SymKind::Alias:
		return sym_qty(builtins, sym_unalias(sym));
	case SymKind::EnumMember:
		// TODO: How to handle when stuct type?
		return QualifiedTy(sym->data.sym_enum_member.struct_ty);
	case SymKind::Namespace:
		return QualifiedTy(builtins.void_ty);
	case SymKind::Invalid:
		return QualifiedTy(builtins.void_ty);
	}
}