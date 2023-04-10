#include "QualifiedTy.h"

#include "Ty.h"

QualifiedTy::QualifiedTy(){};
QualifiedTy::QualifiedTy(Ty const* ty)
	: ty(ty){};
QualifiedTy::QualifiedTy(Ty const* ty, int indirection)
	: ty(ty)
	, indirection(indirection)
{}

// bool
// QualifiedTy::is_array() const
// {
// 	return is_array;
// }

bool
QualifiedTy::is_pointer() const
{
	return indirection != 1;
}

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

QualifiedTy
QualifiedTy::deref() const
{
	assert(indirection != 0);
	return QualifiedTy(ty, indirection - 1);
}

bool
QualifiedTy::equals(QualifiedTy lqty, QualifiedTy rqty)
{
	if( lqty.indirection == rqty.indirection && lqty.ty == rqty.ty )
		return true;

	return false;
}