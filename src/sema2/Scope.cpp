#include "Scope.h"

using namespace sema;

Scope::Scope(Types* tys)
	: types(tys)
{
	for( auto const& ty : types->types )
	{
		Type const* tyc = &ty.second;
		String s = String(ty.first);
		// auto entry = std::make_pair<String, Type const*>(String(ty.first), tyc);
		types_in_scope.emplace(s, tyc);
	}
};
Scope::Scope(Scope* par)
	: parent(par)
	, types(par->types){};

Scope::~Scope()
{
	if( this->expected_return )
	{
		delete this->expected_return;
	}
}

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

TypeInstance const*
Scope::get_expected_return() const
{
	if( expected_return == nullptr )
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
		return expected_return;
	}
}

void
Scope::set_expected_return(TypeInstance n)
{
	expected_return = new TypeInstance(n);
}

Scope*
Scope::get_parent()
{
	return parent;
}
