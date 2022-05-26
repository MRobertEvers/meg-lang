#pragma once
#include "../IExpressionNode.h"
#include "common/OwnPtr.h"
namespace ast
{
enum BinOp : char
{
	plus,
	star,
	minus,
	slash,
	gt,
	gte,
	lt,
	lte,
	and_lex,
	or_lex,
	cmp,
	ne,
	bad
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryOperation : public IExpressionNode
{
public:
	BinOp Op;
	OwnPtr<IExpressionNode> LHS, RHS;
	BinaryOperation(Span span, BinOp Op, OwnPtr<IExpressionNode> LHS, OwnPtr<IExpressionNode> RHS)
		: IExpressionNode(span)
		, Op(Op)
		, LHS(std::move(LHS))
		, RHS(std::move(RHS))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

} // namespace ast