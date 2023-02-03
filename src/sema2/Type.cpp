

#include "Type.h"

using namespace sema;

String
Type::get_name() const
{
	return name;
}

TypeInstance const*
Type::get_member_type(String const& name) const
{
	auto member = members.find(name);
	if( member != members.cend() )
	{
		return &member->second.type;
	}
	else
	{
		return nullptr;
	}
}

TypeInstance const*
Type::get_member_type(int idx) const
{
	if( idx < members_order.size() )
	{
		return &members_order.at(idx).type;
	}
	else
	{
		return nullptr;
	}
}

TypeInstance
Type::get_return_type() const
{
	return return_type;
}

void
Type::add_member(String const& name, TypeInstance type)
{
	members.insert(std::make_pair(name, TypedMember{type, name}));

	members_order.emplace_back(TypedMember{type, name});
}

Type
Type::Function(String const& name, Vec<TypedMember> args, TypeInstance return_type)
{
	return Type{name, args, return_type};
}

Type
Type::Struct(String const& name, std::map<String, TypedMember> members)
{
	return Type{name, members};
}

Type
Type::Primitive(String name)
{
	return Type{name};
}