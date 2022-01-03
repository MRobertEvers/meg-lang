#pragma once

#include "../IStatementNode.h"

#include <string>
#include <vector>

namespace nodes
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

	void codegen() override;
	const std::string& getName() const { return Name; }
};
} // namespace nodes
