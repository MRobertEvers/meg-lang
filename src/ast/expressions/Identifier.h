#pragma once

#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{

// Used to store scoped type information about a type
// e.g.
// let x: MyStruct::NestedStruct, etc.
class Path
{
	Vec<String> path;

public:
	Path(){};
	Path(String& path)
		: path({std::move(path)})
	{}
	Path(Vec<String>& path)
		: path(std::move(path))
	{}
	Path(Path& path)
		: path(std::move(path.path))
	{}

	String get_fqn() const
	{
		String sz;
		unsigned int idx = 0;
		for( auto& s : path )
		{
			if( idx++ != 0 )
				sz += "::";
			sz += s;
		}

		return sz;
	}
};

/**
 * @brief
 * An empty identifier is used for anonymous types and when an optional type specifier is not
 * preset.
 */
class Identifier : public IExpressionNode
{
	Path path;
	bool is_empty_ = true;

public:
	Identifier(String& name)
		: path(name)
		, is_empty_(false)
	{}
	Identifier(String&& name)
		: path(name)
		, is_empty_(false)
	{}

	Identifier(Path& path)
		: path(path)
		, is_empty_(false)
	{}

	virtual String get_fqn() const { return path.get_fqn(); }

	virtual bool is_pointer_type() const { return false; }

protected:
	Identifier(bool is_empty)
		: is_empty_(true)
	{}
};

class TypeIdentifier : public Identifier
{
	OwnPtr<TypeIdentifier> base = nullptr;

public:
	TypeIdentifier(TypeIdentifier&& other)
		: Identifier("")
		, base(std::move(other.base))
	{}
	TypeIdentifier(TypeIdentifier& other)
		: Identifier("")
		, base(std::move(other.base))
	{}

	TypeIdentifier(OwnPtr<TypeIdentifier> base)
		: Identifier("")
		, base(std::move(base))
	{}

	TypeIdentifier(String name)
		: Identifier(name)
	{}

	TypeIdentifier(Path path)
		: Identifier(path)
	{}

	bool is_pointer_type() const override { return !base.is_null(); }

	String get_fqn() const override
	{
		if( base.is_null() )
		{
			return Identifier::get_fqn();
		}
		else
		{
			return base->get_fqn();
		}
	}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeIdentifier()
		: Identifier(true)
	{}

public:
	static OwnPtr<TypeIdentifier> Empty() { return new TypeIdentifier(); }
	static OwnPtr<TypeIdentifier> PointerToTy(OwnPtr<TypeIdentifier> base)
	{
		return OwnPtr<TypeIdentifier>::of(std::move(base));
	}
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
	ValueIdentifier(ValueIdentifier& val)
		: Identifier(val.get_fqn())
	{}
	ValueIdentifier(ValueIdentifier&& val)
		: Identifier(val.get_fqn())
	{}

	ValueIdentifier(String& name)
		: Identifier(name)
	{}
	ValueIdentifier(String&& name)
		: Identifier(name)
	{}
	ValueIdentifier(Path& path)
		: Identifier(path)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

public:
	static OwnPtr<ValueIdentifier> Empty() { return new ValueIdentifier(); }
};

}; // namespace ast