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
	BinaryOperation(char Op, OwnPtr<IExpressionNode> LHS, OwnPtr<IExpressionNode> RHS)
		: Op(Op)
		, LHS(std::move(LHS))
		, RHS(std::move(RHS))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	// TODO: RHS and LHS must have same type.
	Type const& get_type() const override { return RHS->get_type(); }
};
} // namespace ast