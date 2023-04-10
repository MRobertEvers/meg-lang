#include "Sym.h"

SymVar::SymVar(QualifiedTy qty)
	: qty(qty){};

SymFunc::SymFunc(Ty const* ty)
	: ty(ty){};

SymType::SymType(Ty const* ty)
	: ty(ty){};

SymMember::SymMember(QualifiedTy qty, int position)
	: qty(qty)
	, position(position){};

SymEnumMember::SymEnumMember(long long value, Ty const* struct_ty)
	: value(value)
	, struct_ty(struct_ty)
	, kind(MemberKind::Struct){};

SymEnumMember::SymEnumMember(long long value)
	: value(value)
	, struct_ty(nullptr)
	, kind(MemberKind::Simple){};

SymNamespace::SymNamespace(){};
