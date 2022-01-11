#pragma once
#include "../IExpressionNode.h"

namespace ast
{
class Number : public IExpressionNode
{
public:
	int Val;
	Type const& type;
	Number(int Val, Type const& type)
		: Val(Val)
		, type(type)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
	// AstNodeType get_type() const override { return AstNodeType::number; }

	Type const& get_type() const override { return type; }
};

} // namespace ast