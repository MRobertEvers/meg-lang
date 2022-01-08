#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "common/OwnPtr.h"

#include <memory>

namespace ast
{
class Return : public IStatementNode
{
public:
	OwnPtr<IExpressionNode> ReturnExpr;
	Return(OwnPtr<IExpressionNode> ReturnExpr)
		: ReturnExpr(std::move(ReturnExpr))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	// AstNodeType get_type() const override { return AstNodeType::return_node; }
};
} // namespace ast