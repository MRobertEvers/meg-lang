

#pragma once

#include "ir/IR.h"
#include "ir/TypeInstance.h"

namespace ir
{
class RValue
{
public:
	Inst* inst;
	TypeInstance type;

	RValue(Inst* inst, TypeInstance type)
		: inst(inst)
		, type(type){};
};

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

	RValue rvalue() const { return rvalue_; }
	ir::TypeInstance type() const { return type_; }
};
} // namespace ir