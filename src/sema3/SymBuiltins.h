#pragma once

#include "SymTab.h"
#include "Types.h"

class Ast;

struct SymBuiltins
{
	// Primitives
	Ty const* infer_ty;
	Ty const* void_ty;
	Ty const* bool_ty;
	Ty const* ix_ty;
	Ty const* i64_ty;
	Ty const* i32_ty;
	Ty const* i16_ty;
	Ty const* i8_ty;
	Ty const* u64_ty;
	Ty const* u32_ty;
	Ty const* u16_ty;
	Ty const* u8_ty;

private:
	SymBuiltins(){};

public:
	static SymBuiltins create_builtins(SymTab& sym_tab, Types& types, Ast& ast);
};

QualifiedTy sym_qty(SymBuiltins const& builtins, Sym* sym);
Ty const* sym_ty(SymBuiltins const& builtins, Sym* sym);
QualifiedTy int_qty(SymBuiltins const& builtins, int width, bool is_signed);
