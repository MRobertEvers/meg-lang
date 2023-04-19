#include "Expr.h"

Expr::Expr()
	: kind(ExprKind::Empty)
{}

Expr::Expr(RValue rvalue)
	: rvalue_(rvalue)
	, kind(ExprKind::RValue)
{}

Expr::Expr(Address address)
	: address_(address)
	, kind(ExprKind::Address)
{}

bool
Expr::is_void() const
{
	return ExprKind::Empty == kind;
}

bool
Expr::is_rvalue() const
{
	return ExprKind::RValue == kind;
}

bool
Expr::is_address() const
{
	return ExprKind::Address == kind;
}

Address
Expr::address() const
{
	assert(is_address());
	return address_;
}

RValue
Expr::rvalue() const
{
	assert(is_rvalue());
	return rvalue_;
}

Expr
Expr::Empty()
{
	return Expr();
}