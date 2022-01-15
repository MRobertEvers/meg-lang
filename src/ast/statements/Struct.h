#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "../expressions/Identifier.h"
#include "../expressions/TypeDeclarator.h"
#include "common/OwnPtr.h"
#include "common/Vec.h"

namespace ast
{
class MemberVariableDeclaration
{
public:
	Span span;
	OwnPtr<TypeDeclarator> Type;
	OwnPtr<ValueIdentifier> Name;

	MemberVariableDeclaration(Span span, OwnPtr<ValueIdentifier> Name, OwnPtr<TypeDeclarator> Type)
		: span(span)
		, Name(std::move(Name))
		, Type(std::move(Type)){};
};

class Struct : public IStatementNode
{
public:
	Vec<OwnPtr<MemberVariableDeclaration>> MemberVariables;
	OwnPtr<TypeIdentifier> TypeName;

	Struct(
		Span span,
		OwnPtr<TypeIdentifier> TypeName,
		Vec<OwnPtr<MemberVariableDeclaration>> MemberVariables)
		: IStatementNode(span)
		, MemberVariables(std::move(MemberVariables))
		, TypeName(std::move(TypeName))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast