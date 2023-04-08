#include "QualifiedTy.h"

#include "Ty.h"

QualifiedTy::QualifiedTy(){};
QualifiedTy::QualifiedTy(Ty const* ty)
	: ty(ty){};

bool
QualifiedTy::is_function() const
{
	return ty->kind == TyKind::Func;
}

bool
QualifiedTy::is_primitive() const
{
	return ty->kind == TyKind::Primitive && indirection == 0 && !is_array;
}

bool
QualifiedTy::equals(QualifiedTy lqty, QualifiedTy rqty)
{
	if( lqty.indirection == rqty.indirection && lqty.ty == rqty.ty )
		return true;

	return false;
}