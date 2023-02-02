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
	Span span;
	OwnPtr<TypeDeclarator> Type;
	OwnPtr<ValueIdentifier> Name;

	ParameterDeclaration(Span span, OwnPtr<ValueIdentifier> Name, OwnPtr<TypeDeclarator> Type)
		: span(span)
		, Name(std::move(Name))
		, Type(std::move(Type)){};
};

class ParameterList
{
public:
	Span span;
	Vec<OwnPtr<ParameterDeclaration>> Parameters;
	ParameterList(Span span, Vec<OwnPtr<ParameterDeclaration>>& Parms)
		: span(span)
		, Parameters(std::move(Parms)){};
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class Prototype : public IStatementNode
{
public:
	OwnPtr<ParameterList> Parameters;

	// TODO: May be null
	OwnPtr<TypeDeclarator> ReturnType;
	OwnPtr<TypeIdentifier> Name;

	Prototype(
		Span span,
		OwnPtr<TypeIdentifier> Name,
		OwnPtr<TypeDeclarator> ReturnType,
		OwnPtr<ParameterList> Parms)
		: IStatementNode(span)
		, Name(std::move(Name))
		, Parameters(std::move(Parms))
		, ReturnType(std::move(ReturnType)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	const ParameterList* get_parameters() const { return Parameters.get(); }
	bool is_infer_return() const { return ReturnType.is_null(); }
};
} // namespace ast
