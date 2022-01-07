#pragma once

#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>
#include <vector>

namespace ast
{
class MemberVariableDeclaration
{
public:
	std::unique_ptr<TypeIdentifier> Type;
	std::unique_ptr<ValueIdentifier> Name;

	MemberVariableDeclaration(
		std::unique_ptr<ValueIdentifier> Name, std::unique_ptr<TypeIdentifier> Type)
		: Name(std::move(Name))
		, Type(std::move(Type)){};
};

class Struct : public IStatementNode
{
public:
	std::vector<std::unique_ptr<MemberVariableDeclaration>> MemberVariables;
	std::unique_ptr<TypeIdentifier> TypeName;

	Struct(
		std::unique_ptr<TypeIdentifier> TypeName,
		std::vector<std::unique_ptr<MemberVariableDeclaration>> MemberVariables)
		: MemberVariables(std::move(MemberVariables))
		, TypeName(std::move(TypeName))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast