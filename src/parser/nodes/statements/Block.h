#pragma once

#include "../IStatementNode.h"

#include <memory>
#include <vector>

namespace nodes
{
class Block : public IStatementNode
{
	std::vector<std::unique_ptr<IStatementNode>> statements;

public:
	Block(std::vector<std::unique_ptr<IStatementNode>> stmts)
		: statements(std::move(stmts))
	{}

	void codegen() override;
};
} // namespace nodes