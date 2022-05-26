#pragma once
#include "../IStatementNode.h"
#include "common/OwnPtr.h"
namespace ast
{

enum class AssignOp
{
	assign,
	add,
	sub,
	mul,
	div
};

class Assign : public IStatementNode
{
public:
	AssignOp Op;
	OwnPtr<IExpressionNode> lhs, rhs;
	Assign(Span span, AssignOp Op, OwnPtr<IExpressionNode> LHS, OwnPtr<IExpressionNode> RHS)
		: IStatementNode(span)
		, Op(Op)
		, lhs(std::move(LHS))
		, rhs(std::move(RHS))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast