#include "QualifiedTy.h"

QualifiedTy::QualifiedTy(){};
QualifiedTy::QualifiedTy(Ty const* ty)
	: ty(ty){};