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
	std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>> ArgsAndTypes;
	Identifier ReturnType;
	Identifier Name;

	Prototype(
		const Identifier& Name,
		const Identifier& ReturnType,
		std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>>& Args)
		: Name(Name)
		, ArgsAndTypes(std::move(Args))
		, ReturnType(ReturnType){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	const std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>>& get_args() const
	{
		return ArgsAndTypes;
	}
};
} // namespace ast
