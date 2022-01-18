

#include "Type.h"

using namespace sema;

String
Type::get_name() const
{
	if( is_pointer_type() )
	{
		return base->get_name() + "*";
	}
	else
	{
		return name;
	}
}

Type const*
Type::get_member_type(String const& name) const
{
	auto member = members.find(name);
	if( member != members.cend() )
	{
		return member->second;
	}
	else
	{
		return nullptr;
	}
}

void
Type::add_member(String const& name, Type const& type)
{
	members.insert(std::make_pair(name, &type));
}

Type
Type::PointerTo(Type const& base)
{
	return Type{base, true};
}

Type
Type::Function(String const& name, Vec<Type const*> args, Type const* return_type)
{
	return Type{name, args, return_type};
}

Type
Type::Struct(String const& name, std::map<String, Type const*> members)
{
	return Type{name, members};
}

Type
Type::Primitive(String name)
{
	return Type{name};
}