#pragma once

#include "Address.h"
#include "RValue.h"

enum class ExprKind
{
	Empty,
	Address,
	RValue,
};

class Expr
{
	ExprKind kind = ExprKind::Empty;

	union
	{
		RValue rvalue_;
		Address address_;
	};

public:
	Expr();
	Expr(RValue rvalue);
	Expr(Address address);

	bool is_void() const;
	bool is_address() const;
	bool is_rvalue() const;

	Address address() const;
	RValue rvalue() const;

	static Expr Empty();
};