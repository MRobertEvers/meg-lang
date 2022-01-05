#pragma once

#include "../IStatementNode.h"

#include <memory>
#include <vector>

namespace ast
{
class Block : public IStatementNode
{
public:
	std::vector<std::unique_ptr<IStatementNode>> statements;

	Block(std::vector<std::unique_ptr<IStatementNode>> stmts)
		: statements(std::move(stmts))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
	// AstNodeType get_type() const override { return AstNodeType::block; }
};
} // namespace ast