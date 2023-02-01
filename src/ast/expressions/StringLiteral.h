#pragma once
#include "../IExpressionNode.h"

namespace ast
{
class StringLiteral : public IExpressionNode
{
public:
	String Val;
	StringLiteral(Span span, String Val)
		: IExpressionNode(span)
		, Val(Val)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

} // namespace ast