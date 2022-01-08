#pragma once

#include <memory>
#include <string>
#include <vector>

namespace ast
{

/**
 * @brief
 * An empty identifier is used for anonymous types and when an optional type specifier is not
 * preset.
 *
 */
class Identifier : public IExpressionNode
{
public:
	bool is_empty = true;
	// Path shit only works for value types right now.
	std::vector<std::string> path;
	Identifier(const std::string& name)
		: is_empty(false)
	{
		path.emplace_back(name);
	}

	Identifier(std::vector<std::string>& path)
		: path(path)
		, is_empty(false)
	{}

	virtual std::string get_fqn() const
	{
		std::string sz;
		unsigned int idx = 0;
		for( auto& s : path )
		{
			if( idx++ != 0 )
				sz += ".";
			sz += s;
		}
		return sz;
	}

	virtual bool is_pointer_type() const { return false; }

	std::string get_element_name() const
	{
		if( path.size() == 0 )
		{
			return "";
		}
		else
		{
			return *path.rbegin();
		}
	}

protected:
	Identifier(bool is_empty)
		: is_empty(true)
	{}
};

class TypeIdentifier : public Identifier
{
	std::unique_ptr<TypeIdentifier> base = nullptr;

public:
	TypeIdentifier(const std::string& name)
		: Identifier(name)
	{}

	TypeIdentifier(std::vector<std::string>& path)
		: Identifier(path)
	{}

	TypeIdentifier(std::unique_ptr<TypeIdentifier> base)
		: Identifier(std::string{"*"})
		, base(std::move(base))
	{}

	bool is_pointer_type() const override { return base.get() != nullptr; }

	std::string get_fqn() const override
	{
		// return Identifier::get_fqn() + (base.get() == nullptr ? "" : "*");
		if( is_pointer_type() )
		{
			return base->get_fqn();
		}
		else
		{
			return Identifier::get_fqn();
		}
	}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeIdentifier()
		: Identifier(true)
	{}

public:
	static TypeIdentifier Empty() { return TypeIdentifier(); }
	static TypeIdentifier PointerToTy(std::unique_ptr<TypeIdentifier> base)
	{
		return TypeIdentifier(std::move(base));
	}
};

/// Ripped off from llvm
/// Represent the declaration of a variable (in which case it is
/// an lvalue) a function (in which case it is a function designator) or
/// an enum constant.
class ValueIdentifier : public Identifier
{
public:
	ValueIdentifier(const std::string& name)
		: Identifier(name)
	{}
	ValueIdentifier(std::vector<std::string>& path)
		: Identifier(path)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	ValueIdentifier()
		: Identifier(true)
	{}

public:
	static ValueIdentifier Empty() { return ValueIdentifier(); }
};

}; // namespace ast