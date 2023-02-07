#include "Scope.h"
using namespace cg;

Scope::Scope(Scope* parent)
	: parent(parent)
{}

void
Scope::add_lvalue(String const& name, LValue lvalue)
{
	values.emplace(name, lvalue);
}

std::optional<LValue>
Scope::lookup(String const& name) const
{
	auto iter_values = values.find(name);
	if( iter_values != values.end() )
	{
		return iter_values->second;
	}
	else
	{
		if( parent )
			return parent->lookup(name);
		else
			return std::optional<LValue>();
	}
}

Scope*
Scope::get_parent() const
{
	return this->parent;
}