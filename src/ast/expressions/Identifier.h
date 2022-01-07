#pragma once

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

	std::string get_fqn() const
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
public:
	TypeIdentifier(const std::string& name)
		: Identifier(name)
	{}
	TypeIdentifier(std::vector<std::string>& path)
		: Identifier(path)
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeIdentifier()
		: Identifier(true)
	{}

public:
	static TypeIdentifier Empty() { return TypeIdentifier(); }
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