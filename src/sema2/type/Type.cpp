

#include "Type.h"

using namespace sema;

Type::Type(String name)
	: name(name)
	, cls(TypeClassification::primitive){};

Type::Type(String name, std::map<String, TypedMember> members)
	: name(name)
	, members(members)
	, cls(TypeClassification::struct_cls){};

Type::Type(String name, Vec<TypedMember> args, TypeInstance return_type)
	: name(name)
	, members_order(args)
	, return_type(return_type)
	, cls(TypeClassification::function)
{
	for( auto arg : args )
	{
		members.emplace(arg.name, arg);
	}
};

Type::~Type()
{
	// switch( cls )
	// {
	// case TypeClassification::function:
	// 	this->type_info.fn_.~FunctionTypeInfo();
	// 	break;
	// case TypeClassification::struct_cls:
	// 	this->type_info.struct_.~StructTypeInfo();
	// 	break;
	// case TypeClassification::primitive:
	// 	break;
	// }
}

String
Type::get_name() const
{
	return name;
}

// static TypeInstance const*
// find(std::map<String, TypedMember>& members, String& name)
// {
// 	auto member = members.find(name);
// 	if( member != members.cend() )
// 	{
// 		return &member->second.type;
// 	}
// 	else
// 	{
// 		return nullptr;
// 	}
// }

// TypeInstance const*
// Type::get_member_type(String const& name) const
// {
// 	switch( cls )
// 	{
// 	case TypeClassification::primitive:
// 		return nullptr;
// 	case TypeClassification::struct_cls:
// 		return find(this->type_info.struct_.members, name);
// 	case TypeClassification::function:
// 		return find(this->type_info.fn_.members, name);
// 	default:
// 		break;
// 	}
// }

// TypeInstance const*
// Type::get_member_type(int idx) const
// {
// 	if( cls != TypeClassification::function )
// 		return nullptr;

// 	if( idx < this->type_info.fn_.members_order.size() )
// 	{
// 		return &this->type_info.fn_.members_order.at(idx).type;
// 	}
// 	else
// 	{
// 		return nullptr;
// 	}
// }

// TypeInstance
// Type::get_return_type() const
// {
// 	return return_type;
// }

// void
// Type::add_member(String const& name, TypeInstance type)
// {
// 	members.insert(std::make_pair(name, TypedMember{type, name}));

// 	members_order.emplace_back(TypedMember{type, name});
// }

std::optional<TypeInstance>
Type::get_return_type() const
{
	if( cls != TypeClassification::function )
		return std::optional<TypeInstance>();

	return this->return_type;
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