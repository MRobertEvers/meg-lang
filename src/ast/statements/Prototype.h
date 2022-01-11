#pragma once

#include "../IStatementNode.h"
#include "../expressions/Identifier.h"
#include "../expressions/TypeDeclarator.h"
#include "common/OwnPtr.h"
#include "common/Vec.h"

#include <string>
#include <vector>
namespace ast
{

class ParameterDeclaration
{
public:
	OwnPtr<TypeDeclarator> Type;
	OwnPtr<ValueIdentifier> Name;

	ParameterDeclaration(OwnPtr<ValueIdentifier> Name, OwnPtr<TypeDeclarator> Type)
		: Name(std::move(Name))
		, Type(std::move(Type)){};
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class Prototype : public IStatementNode
{
public:
	Vec<OwnPtr<ParameterDeclaration>> Parameters;
	OwnPtr<TypeDeclarator> ReturnType;
	OwnPtr<TypeIdentifier> Name;

	Prototype(
		OwnPtr<TypeIdentifier> Name,
		OwnPtr<TypeDeclarator> ReturnType,
		Vec<OwnPtr<ParameterDeclaration>>& Parms)
		: Name(std::move(Name))
		, Parameters(std::move(Parms))
		, ReturnType(std::move(ReturnType)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	const Vec<OwnPtr<ParameterDeclaration>>& get_parameters() const { return Parameters; }
};
} // namespace ast
