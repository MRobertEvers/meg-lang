#pragma once

#include "../IStatementNode.h"
#include "../expressions/Identifier.h"

#include <string>
#include <vector>

namespace ast
{

class ParameterDeclaration
{
public:
	std::unique_ptr<TypeIdentifier> Type;
	std::unique_ptr<ValueIdentifier> Name;

	ParameterDeclaration(
		std::unique_ptr<ValueIdentifier> Name, std::unique_ptr<TypeIdentifier> Type)
		: Name(std::move(Name))
		, Type(std::move(Type)){};
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class Prototype : public IStatementNode
{
public:
	std::vector<std::unique_ptr<ParameterDeclaration>> Parameters;
	std::unique_ptr<TypeIdentifier> ReturnType;
	std::unique_ptr<ValueIdentifier> Name;

	Prototype(
		std::unique_ptr<ValueIdentifier> Name,
		std::unique_ptr<TypeIdentifier> ReturnType,
		std::vector<std::unique_ptr<ParameterDeclaration>>& Parms)
		: Name(std::move(Name))
		, Parameters(std::move(Parms))
		, ReturnType(std::move(ReturnType)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	const std::vector<std::unique_ptr<ParameterDeclaration>>& get_parameters() const
	{
		return Parameters;
	}
};
} // namespace ast
