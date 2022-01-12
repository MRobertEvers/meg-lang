#pragma once
#include "../IStatementNode.h"
#include "common/OwnPtr.h"
namespace ast
{

class Assign : public IStatementNode
{
public:
	char Op;
	OwnPtr<IExpressionNode> lhs, rhs;
	Assign(char Op, OwnPtr<IExpressionNode> LHS, OwnPtr<IExpressionNode> RHS)
		: Op(Op)
		, lhs(std::move(LHS))
		, rhs(std::move(RHS))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast