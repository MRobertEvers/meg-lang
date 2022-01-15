#pragma once

#include "IStatementNode.h"
#include "common/OwnPtr.h"
#include "common/Vec.h"

namespace ast
{
class Module : public IAstNode
{
public:
	Vec<OwnPtr<IStatementNode>> statements;

	Module(Span span, Vec<OwnPtr<IStatementNode>> statements)
		: IAstNode(span)
		, statements(std::move(statements)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

} // namespace ast