#pragma once

#include "../IStatementNode.h"
#include "Prototype.h"

#include <memory>

namespace ast
{

/// FunctionAST - This class represents a function definition itself.
class Function : public IStatementNode
{
public:
	std::unique_ptr<Prototype> Proto;
	std::unique_ptr<Block> Body;

	Function(std::unique_ptr<Prototype> Proto, std::unique_ptr<Block> Body)
		: Proto(std::move(Proto))
		, Body(std::move(Body))
	{}

	// AstNodeType get_type() const override { return AstNodeType::fn; }
	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast