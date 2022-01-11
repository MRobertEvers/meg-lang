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
		: path({path})
	{}
	Path(String&& path)
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
				sz += ".";
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
protected:
	Path path;

public:
	Identifier(String& name)
		: path(name)
	{}
	Identifier(String&& name)
		: path(name)
	{}

	Identifier(Path& path)
		: path(path)
	{}

	virtual String get_fqn() const { return path.get_fqn(); }

	virtual bool is_type_identifier() const { return false; }

protected:
	Identifier(bool is_empty) {}
};

class TypeIdentifier : public Identifier
{
	TypeIdentifier const* base = nullptr;
	OwnPtr<Type> type = void_type;

public:
	// Copy and move constructors
	TypeIdentifier(TypeIdentifier&& other)
		: Identifier(other.path)
		, base(std::move(other.base))
		, type(std::move(other.type))
	{}
	TypeIdentifier(TypeIdentifier& other)
		: Identifier(other.path)
		, base(std::move(other.base))
		, type(std::move(other.type))
	{}

	// Base constructors
	TypeIdentifier(String name)
		: Identifier(name)
		, type(new Type{name})
	{}

	// Constructor of pointer to type
	TypeIdentifier(TypeIdentifier const* base)
		: Identifier("")
		, base(base)
		// base arg is moved before this type (based on order of declaration; be careful!)
		, type(Type::PointerTo(*this->base->type.get()))
	{}

	// TODO: not sure path is needed
	/**
	 * @brief Construct a new Type Identifier object
	 *
	 * @deprecated dont use path
	 *
	 * @param path
	 */
	TypeIdentifier(Path path)
		: Identifier(path)
		, type(new Type{path.get_fqn()})
	{}

	bool is_pointer_type() const { return base != nullptr; }
	bool is_type_identifier() const override { return true; }

	String get_fqn() const override
	{
		if( base == nullptr )
		{
			return Identifier::get_fqn();
		}
		else
		{
			return base->get_fqn();
		}
	}

	Type const& get_type() const override { return *type.get(); }

	void add_member(String const& name, Type const& type) { this->type->add_member(name, type); }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeIdentifier()
		: Identifier(true)
		, type(infer_type)
	{}

public:
	// TODO: This should be anonymous.
	static OwnPtr<TypeIdentifier> Empty() { return new TypeIdentifier(); }
	// static OwnPtr<TypeIdentifier> PointerToTy(OwnPtr<TypeIdentifier> base)
	// {
	// 	return OwnPtr<TypeIdentifier>::of(std::move(base));
	// }
	// static OwnPtr<TypeIdentifier> PointerToTy(TypeIdentifier const* base)
	// {
	// 	return OwnPtr<TypeIdentifier>::of(base);
	// }
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
		, type(void_type)
	{}

	Type const& type;

public:
	ValueIdentifier(ValueIdentifier& other)
		: Identifier(other.get_fqn())
		, type(other.type)
	{}
	ValueIdentifier(ValueIdentifier&& other)
		: Identifier(other.get_fqn())
		, type(other.type)
	{}

	ValueIdentifier(String& name, Type const& type)
		: Identifier(name)
		, type(type)
	{}
	ValueIdentifier(String&& name, Type const& type)
		: Identifier(name)
		, type(type)
	{}
	ValueIdentifier(Path& path, Type const& type)
		: Identifier(path)
		, type(type)
	{}
	ValueIdentifier(Path&& path, Type const& type)
		: Identifier(path)
		, type(type)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

	Type const& get_type() const override { return type; }

public:
	static OwnPtr<ValueIdentifier> Empty() { return new ValueIdentifier(); }
};

}; // namespace ast