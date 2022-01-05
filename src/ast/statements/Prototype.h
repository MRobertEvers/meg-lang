#pragma once

#include "../IStatementNode.h"
#include "../Identifier.h"

#include <string>
#include <vector>

namespace ast
{

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class Prototype : public IStatementNode
{
public:
	std::vector<std::unique_ptr<Identifier>> ArgsAndTypes;

	Identifier Name;
	Prototype(const Identifier& Name, std::vector<std::unique_ptr<Identifier>>& Args)
		: Name(Name)
		, ArgsAndTypes(std::move(Args))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	const std::vector<std::unique_ptr<Identifier>>& get_args() const { return ArgsAndTypes; }
};
} // namespace ast
