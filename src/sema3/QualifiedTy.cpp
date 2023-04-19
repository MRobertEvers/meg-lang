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
	return ty->kind == TyKind::Int && indirection == 0 && !is_array_;
}

bool
QualifiedTy::is_pointer() const
{
	return indirection != 0;
}

bool
QualifiedTy::is_function() const
{
	return ty->kind == TyKind::Func && indirection == 0 && !is_array_;
}

bool
QualifiedTy::is_enum() const
{
	return ty->kind == TyKind::Enum && indirection == 0 && !is_array_;
}

bool
QualifiedTy::is_struct() const
{
	return ty->kind == TyKind::Struct && indirection == 0 && !is_array_;
}

bool
QualifiedTy::is_union() const
{
	return ty->kind == TyKind::Union && indirection == 0 && !is_array_;
}

bool
QualifiedTy::is_subscriptable() const
{
	return is_pointer() || is_array_;
}

bool
QualifiedTy::is_aggregate_type() const
{
	if( indirection != 0 )
		return false;

	return is_enum() || is_struct() || is_union() || is_array_;
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
	if( lqty.indirection == rqty.indirection && lqty.ty == rqty.ty &&
		lqty.is_array_ == rqty.is_array_ )
		return true;

	return false;
}