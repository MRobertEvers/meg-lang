#pragma once
#include "IAstVisitor.h"
#include "Span.h"

namespace ast
{
class IAstNode
{
	Span span;

public:
	IAstNode(Span span)
		: span(span){};
	virtual ~IAstNode() = default;

	virtual void visit(IAstVisitor* visitor) const = 0;

	Span get_span() const { return span; }
};
} // namespace ast