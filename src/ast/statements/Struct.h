#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"
#include "common/OwnPtr.h"
#include "common/Vec.h"

namespace ast
{
class MemberVariableDeclaration
{
public:
	OwnPtr<TypeIdentifier> Type;
	OwnPtr<ValueIdentifier> Name;

	MemberVariableDeclaration(OwnPtr<ValueIdentifier> Name, OwnPtr<TypeIdentifier> Type)
		: Name(std::move(Name))
		, Type(std::move(Type)){};
};

class Struct : public IStatementNode
{
public:
	Vec<OwnPtr<MemberVariableDeclaration>> MemberVariables;
	OwnPtr<TypeIdentifier> TypeName;

	Struct(OwnPtr<TypeIdentifier> TypeName, Vec<OwnPtr<MemberVariableDeclaration>> MemberVariables)
		: MemberVariables(std::move(MemberVariables))
		, TypeName(std::move(TypeName))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast