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
	// AstNodeType get_type() const override { return AstNodeType::bin_op; }
};
} // namespace ast