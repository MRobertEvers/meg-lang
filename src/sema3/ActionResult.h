

#pragma once

#include "ir/TypeInstance.h"

namespace ir
{
class RValue
{};

class LValue
{};

class ActionResult
{
	enum class Kind
	{
		Void,
		LValue,
		RValue,
		Type
	};

	union
	{
		LValue lvalue_;
		RValue rvalue_;
		ir::TypeInstance type_;
	};

	Kind kind = Kind::Void;

public:
	ActionResult(LValue lvalue);
	ActionResult(RValue rvalue);
	ActionResult(ir::TypeInstance type);
	ActionResult();

	bool is_type() const { return kind == Kind::Type; }
	bool is_void() const { return kind == Kind::Void; }

	ir::TypeInstance type() const { return type_; }
};
} // namespace ir