#pragma once

#include "../IStatementNode.h"
#include "common/OwnPtr.h"
#include "common/Vec.h"

namespace ast
{
class Block : public IStatementNode
{
public:
	Vec<OwnPtr<IStatementNode>> statements;
	Block()
		: IStatementNode(Span{}){};
	Block(Span span, Vec<OwnPtr<IStatementNode>> stmts)
		: IStatementNode(span)
		, statements(std::move(stmts))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
	// AstNodeType get_type() const override { return AstNodeType::block; }
};
} // namespace ast