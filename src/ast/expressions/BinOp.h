#pragma once
#include "../IExpressionNode.h"
#include "common/OwnPtr.h"
namespace ast
{
/// BinaryExprAST - Expression class for a binary operator.
class BinaryOperation : public IExpressionNode
{
public:
	char Op;
	OwnPtr<IExpressionNode> LHS, RHS;
	BinaryOperation(Span span, char Op, OwnPtr<IExpressionNode> LHS, OwnPtr<IExpressionNode> RHS)
		: IExpressionNode(span)
		, Op(Op)
		, LHS(std::move(LHS))
		, RHS(std::move(RHS))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast