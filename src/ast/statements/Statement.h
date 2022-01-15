#pragma once
#include "../IStatementNode.h"
#include "common/OwnPtr.h"
namespace ast
{

/**
 * @brief Doesn't really do anything; used in parsing for faithful pretty printing.
 *
 */
class Statement : public IStatementNode
{
public:
	OwnPtr<IStatementNode> stmt;
	Statement(Span span, OwnPtr<IStatementNode> stmt)
		: IStatementNode(span)
		, stmt(std::move(stmt))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast