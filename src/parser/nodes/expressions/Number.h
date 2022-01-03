#pragma once
#include "../IExpressionNode.h"

namespace nodes
{
class Number : public IExpressionNode
{
	int Val;

public:
	Number(int Val)
		: Val(Val)
	{}

	llvm::Value* codegen() override;
};

} // namespace nodes