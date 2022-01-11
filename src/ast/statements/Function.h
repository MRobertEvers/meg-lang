#pragma once

#include "../IStatementNode.h"
#include "Prototype.h"

namespace ast
{

/// FunctionAST - This class represents a function definition itself.
class Function : public IStatementNode
{
public:
	OwnPtr<Prototype> Proto;
	OwnPtr<Block> Body;

	Function(OwnPtr<Prototype> Proto, OwnPtr<Block> Body)
		: Proto(std::move(Proto))
		, Body(std::move(Body))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast