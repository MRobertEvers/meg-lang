#pragma once
#include "../IExpressionNode.h"

#include <memory>

namespace ast
{
/// BinaryExprAST - Expression class for a binary operator.
class BinaryOperation : public IExpressionNode
{
	char Op;
	std::unique_ptr<IExpressionNode> LHS, RHS;

public:
	BinaryOperation(
		char Op, std::unique_ptr<IExpressionNode> LHS, std::unique_ptr<IExpressionNode> RHS)
		: Op(Op)
		, LHS(std::move(LHS))
		, RHS(std::move(RHS))
	{}

	llvm::Value* codegen(CodegenContext& codegen) override;
};
} // namespace ast