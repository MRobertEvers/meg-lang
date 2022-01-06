#pragma once

#include <string>

namespace ast
{

class Identifier : public IExpressionNode
{
public:
	bool is_empty = true;
	std::string name;
	Identifier(const std::string& name)
		: name(name)
		, is_empty(false)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	static Identifier Empty() { return Identifier(true); }

private:
	Identifier(bool is_empty)
		: name("")
		, is_empty(true)
	{}
};

}; // namespace ast