#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>

namespace nodes
{
class Return : public IStatementNode
{
	std::unique_ptr<IExpressionNode> ReturnExpr;

public:
	Return(std::unique_ptr<IExpressionNode> ReturnExpr)
		: ReturnExpr(std::move(ReturnExpr))
	{}

	void codegen() override;
};
} // namespace nodes