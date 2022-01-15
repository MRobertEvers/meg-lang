#pragma once
#include "../IExpressionNode.h"

namespace ast
{
class Number : public IExpressionNode
{
public:
	int Val;
	Number(int Val)
		: Val(Val)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

} // namespace ast