#pragma once

#include <string>

namespace ast
{

class Identifier : public IExpressionNode
{
public:
	std::string name;
	Identifier(const std::string& name)
		: name(name)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};

}; // namespace ast