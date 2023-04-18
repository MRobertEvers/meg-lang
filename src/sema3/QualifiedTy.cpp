#include "QualifiedTy.h"

#include "Ty.h"

QualifiedTy::QualifiedTy(){};
QualifiedTy::QualifiedTy(Ty const* ty)
	: ty(ty){};
QualifiedTy::QualifiedTy(Ty const* ty, int indirection)
	: ty(ty)
	, indirection(indirection)
{}
QualifiedTy::QualifiedTy(Ty const* ty, ImplKind kind)
	: ty(ty)
	, impl(kind){};
// bool
// QualifiedTy::is_array() const
// {
// 	return is_array;
// }

bool
QualifiedTy::is_int() const
{
	return ty->kind == TyKind::Int && indirection == 0 && !is_array;
}

bool
QualifiedTy::is_pointer() const
{
	return indirection != 1;
}

bool
QualifiedTy::is_function() const
{
	return ty->kind == TyKind::Func && indirection == 0 && !is_array;
}

bool
QualifiedTy::is_enum() const
{
	return ty->kind == TyKind::Enum && indirection == 0 && !is_array;
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