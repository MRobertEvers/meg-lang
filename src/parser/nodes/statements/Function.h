#pragma once

#include "../IStatementNode.h"
#include "Prototype.h"

#include <memory>

namespace nodes
{

/// FunctionAST - This class represents a function definition itself.
class Function : public IStatementNode
{
	std::unique_ptr<Prototype> Proto;
	std::unique_ptr<IStatementNode> Body;

public:
	Function(std::unique_ptr<Prototype> Proto, std::unique_ptr<IStatementNode> Body)
		: Proto(std::move(Proto))
		, Body(std::move(Body))
	{}

	void codegen(CodegenContext& codegen) override;
};
} // namespace nodes