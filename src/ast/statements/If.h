#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "Block.h"
#include "Prototype.h"

namespace ast
{
class If : public IStatementNode
{
public:
	OwnPtr<IExpressionNode> condition;
	OwnPtr<IStatementNode> then_block;
	OwnPtr<IStatementNode> else_block;

	If(Span span, OwnPtr<IExpressionNode> condition, OwnPtr<IStatementNode> then_block)
		: IStatementNode(span)
		, condition(std::move(condition))
		, then_block(std::move(then_block))
		, else_block(nullptr){};

	If(Span span,
	   OwnPtr<IExpressionNode> condition,
	   OwnPtr<IStatementNode> then_block,
	   OwnPtr<IStatementNode> else_block)
		: IStatementNode(span)
		, condition(std::move(condition))
		, then_block(std::move(then_block))
		, else_block(std::move(else_block)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast