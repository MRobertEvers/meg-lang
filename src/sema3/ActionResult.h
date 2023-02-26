

#pragma once

#include "ir/IR.h"
#include "ir/TypeInstance.h"

namespace ir
{
class Action
{
public:
	Inst* inst;
	TypeInstance type;

	Action(Inst* inst, TypeInstance type)
		: inst(inst)
		, type(type){};
};

class ActionResult
{
	enum class Kind
	{
		Void,
		Action,
		Type
	};

	union
	{
		Action action_;
		ir::TypeInstance type_;
	};

	Kind kind = Kind::Void;

public:
	ActionResult(Action rvalue);
	ActionResult(ir::TypeInstance type);
	ActionResult();

	bool is_type() const { return kind == Kind::Type; }
	bool is_void() const { return kind == Kind::Void; }

	Action action() const { return action_; }
	ir::TypeInstance type() const { return type_; }
};
} // namespace ir