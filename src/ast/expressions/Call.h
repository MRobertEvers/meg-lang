
#pragma once

#include "../IExpressionNode.h"
#include "Identifier.h"
#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{

class ArgumentList
{
public:
	Span span;
	Vec<OwnPtr<IExpressionNode>> args;

	ArgumentList(Span span, Vec<OwnPtr<IExpressionNode>>& args)
		: span(span)
		, args(std::move(args)){};
};

/**
 * @brief a()
 *
 */
class Call : public IExpressionNode
{
public:
	ArgumentList args;
	OwnPtr<IExpressionNode> call_target = nullptr;

	Call(Span span, OwnPtr<IExpressionNode> call_target, ArgumentList& args)
		: IExpressionNode(span)
		, call_target(std::move(call_target))
		, args(std::move(args))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast