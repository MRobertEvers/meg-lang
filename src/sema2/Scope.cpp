#include "Scope.h"

using namespace sema;

Scope::Scope(Types* ty)
	: types(ty)
	, parent(nullptr)
{}

Scope::Scope(Scope* par)
	: types(par->types)
	, parent(par){};

void
Scope::add_value_identifier(String const& name, TypeInstance id)
{
	auto iter = names.emplace(String{name}, id);
}

void
Scope::add_type_identifier(Type const* id)
{
	auto iter = types_in_scope.emplace(id->get_name(), id);
}

Type const*
Scope::lookup_type(String const& name) const
{
	auto me = types_in_scope.find(name);
	if( me == types_in_scope.end() )
	{
		if( parent == nullptr )
		{
			return nullptr;
		}
		else
		{
			return parent->lookup_type(name);
		}
	}
	else
	{
		return me->second;
	}
}

TypeInstance const*
Scope::lookup_value_type(String const& name) const
{
	auto me = names.find(name);
	if( me == names.end() )
	{
		if( parent == nullptr )
		{
			return nullptr;
		}
		else
		{
			return parent->lookup_value_type(name);
		}
	}
	else
	{
		return &me->second;
	}
}

std::optional<TypeInstance>
Scope::get_expected_return() const
{
	if( !expected_return.has_value() )
	{
		if( parent == nullptr )
		{
			return std::optional<TypeInstance>();
		}
		else
		{
			return parent->get_expected_return();
		}
	}
	else
	{
		return expected_return;
	}
}

void
Scope::set_expected_return(TypeInstance n)
{
	expected_return = n;
}

void
Scope::clear_expected_return()
{
	expected_return = std::optional<TypeInstance>();
}

Scope*
Scope::get_parent()
{
	return parent;
}
