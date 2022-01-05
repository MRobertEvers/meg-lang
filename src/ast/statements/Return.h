#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>

namespace ast
{
class Return : public IStatementNode
{
public:
	std::unique_ptr<IExpressionNode> ReturnExpr;
	Return(std::unique_ptr<IExpressionNode> ReturnExpr)
		: ReturnExpr(std::move(ReturnExpr))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	// AstNodeType get_type() const override { return AstNodeType::return_node; }
};
} // namespace ast