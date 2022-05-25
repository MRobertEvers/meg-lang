#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "Prototype.h"

namespace ast
{

class For : public IStatementNode
{
public:
	OwnPtr<IStatementNode> init;
	OwnPtr<IExpressionNode> condition;
	OwnPtr<IStatementNode> end_loop;
	OwnPtr<IStatementNode> body;

	For(Span span,
		OwnPtr<IStatementNode> init,
		OwnPtr<IExpressionNode> condition,
		OwnPtr<IStatementNode> end_loop,
		OwnPtr<IStatementNode> body)
		: IStatementNode(span)
		, init(std::move(init))
		, condition(std::move(condition))
		, end_loop(std::move(end_loop))
		, body(std::move(body))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast