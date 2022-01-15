#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "common/OwnPtr.h"

namespace ast
{
class While : public IStatementNode
{
public:
	OwnPtr<IExpressionNode> condition;
	OwnPtr<IStatementNode> loop_block;

	While(Span span, OwnPtr<IExpressionNode> condition, OwnPtr<IStatementNode> loop_block)
		: IStatementNode(span)
		, condition(std::move(condition))
		, loop_block(std::move(loop_block)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast