#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{

/**
 * @brief
 * An empty identifier is used for anonymous types and when an optional type specifier is not
 * preset.
 */
class Identifier : public IExpressionNode
{
protected:
	String name;

public:
	Identifier(String& name)
		: name(name)
	{}
	Identifier(String&& name)
		: name(name)
	{}

	virtual String get_name() const { return name; }

	virtual bool is_type_identifier() const { return false; }

protected:
	Identifier(bool is_empty) {}
};

class TypeIdentifier : public Identifier
{
public:
	// Base constructors
	TypeIdentifier(String name)
		: Identifier(name)
	{}

	bool is_type_identifier() const override { return true; }

	bool is_empty() const { return !name.empty(); }

	String get_name() const override { return Identifier::get_name(); }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeIdentifier()
		: Identifier(true)
	{}

public:
	static OwnPtr<TypeIdentifier> Empty() { return new TypeIdentifier(); }
};

/// Ripped off from llvm
/// Represent the declaration of a variable (in which case it is
/// an lvalue) a function (in which case it is a function designator) or
/// an enum constant.
/// This is only ever a single identifier, e.g.
/// let name = 5;
class ValueIdentifier : public Identifier
{
private:
	ValueIdentifier()
		: Identifier(true)
	{}

public:
	ValueIdentifier(ValueIdentifier& other)
		: Identifier(other.get_name())
	{}
	ValueIdentifier(ValueIdentifier&& other)
		: Identifier(other.get_name())
	{}

	ValueIdentifier(String& name)
		: Identifier(name)
	{}
	ValueIdentifier(String&& name)
		: Identifier(name)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

public:
	static OwnPtr<ValueIdentifier> Empty() { return new ValueIdentifier(); }
};

}; // namespace ast