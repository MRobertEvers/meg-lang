#pragma once

#include "IStatementNode.h"

#include <memory>
#include <vector>

namespace ast
{
class Module : public IAstNode
{
public:
	std::vector<std::unique_ptr<IStatementNode>> statements;

	Module(std::vector<std::unique_ptr<IStatementNode>> statements)
		: statements(std::move(statements)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

} // namespace ast