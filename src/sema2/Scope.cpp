#include "Scope.h"

using namespace sema;

Scope::Scope(){};
Scope::Scope(Scope* parent)
	: parent(parent){};

Scope::~Scope()
{
	//
}

void
Scope::add_value_identifier(String const& name, TypeInstance id)
{
	auto iter = names.emplace(String{name}, id);
}

void
Scope::add_type_identifier(Type const* id)
{
	auto iter = types.emplace(id->get_name(), id);
}

Type const*
Scope::lookup_type(String const& name) const
{
	auto me = types.find(name);
	if( me == types.end() )
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

TypeInstance const*
Scope::get_expected_return() const
{
	if( expected_return.type == nullptr )
	{
		if( parent == nullptr )
		{
			return nullptr;
		}
		else
		{
			return parent->get_expected_return();
		}
	}
	else
	{
		return &expected_return;
	}
}

void
Scope::set_expected_return(TypeInstance n)
{
	expected_return = n;
}

Scope*
Scope::get_parent()
{
	return parent;
}
