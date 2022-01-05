#pragma once

#include "../IStatementNode.h"

#include <string>
#include <vector>

namespace ast
{

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class Prototype : public IStatementNode
{
	std::string Name;
	std::vector<std::string> Args;

public:
	Prototype(const std::string& Name, std::vector<std::string> Args)
		: Name(Name)
		, Args(std::move(Args))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	// AstNodeType get_type() const override { return AstNodeType::prototype; }
	const std::string& get_name() const { return Name; }
	const std::vector<std::string>& get_args() const { return Args; }
};
} // namespace ast
