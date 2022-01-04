#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>

namespace ast
{
class Return : public IStatementNode
{
	std::unique_ptr<IExpressionNode> ReturnExpr;

public:
	Return(std::unique_ptr<IExpressionNode> ReturnExpr)
		: ReturnExpr(std::move(ReturnExpr))
	{}

	void codegen(CodegenContext& codegen) override;
};
} // namespace ast