#pragma once

#include "SymTab.h"
#include "Types.h"

struct SymBuiltins
{
	Ty const* void_ty;
	Ty const* bool_ty;
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
	static SymBuiltins create_builtins(SymTab& sym_tab, Types& types);
};

QualifiedTy sym_qty(SymBuiltins const& builtins, Sym* sym);
