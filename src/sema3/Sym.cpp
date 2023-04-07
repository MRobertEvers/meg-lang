#include "Sym.h"

SymVar::SymVar(QualifiedTy qty)
	: qty(qty){};

SymFunc::SymFunc(Ty const* ty)
	: ty(ty){};

SymType::SymType(Ty const* ty)
	: ty(ty){};

SymNamespace::SymNamespace(){};
