
#pragma once

#include "../IExpressionNode.h"
#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{
/**
 * @brief Doesn't really do anything; used in parsing for faithful pretty printing.
 *
 */
class Expression : public IExpressionNode
{
public:
	OwnPtr<IExpressionNode> base = nullptr;

	Expression(Span span, OwnPtr<IExpressionNode> base)
		: IExpressionNode(span)
		, base(std::move(base))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast