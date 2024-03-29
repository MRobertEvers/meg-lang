

#include "Type.h"

using namespace sema;

Type::Type(String name)
	: name(name)
	, is_var_arg_(false)
	, cls(TypeClassification::primitive){};

Type::Type(String name, std::map<String, MemberTypeInstance> members, TypeClassification cls)
	: name(name)
	, members(members)
	, is_var_arg_(false)
	, cls(cls)
{
	// TODO: Ensure idx matches order.
	for( auto [name, member] : members )
	{
		members_order.push_back(member);
	}
};

Type::Type(String name, Vec<MemberTypeInstance> args, TypeInstance return_type, bool is_var_arg)
	: name(name)
	, members_order(args)
	, return_type(return_type)
	, is_var_arg_(is_var_arg)
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

EnumNominal
Type::as_nominal() const
{
	assert(nominal_.has_value());
	return nominal_.value();
}

std::optional<MemberTypeInstance>
Type::get_member(String const& name) const
{
	auto iter_member = members.find(name);
	if( iter_member != members.end() )
		return iter_member->second;

	return std::optional<MemberTypeInstance>();
}

MemberTypeInstance
Type::get_member(int idx) const
{
	assert(idx < members_order.size());

	return members_order.at(idx);
}

int
Type::get_member_count() const
{
	if( cls == TypeClassification::function )
		return members_order.size();
	else
		return members.size();
}

void
Type::set_enum_members(std::map<String, MemberTypeInstance> members)
{
	this->members = members;
	for( auto member : members )
		members_order.push_back(member.second);
}

std::optional<TypeInstance>
Type::get_return_type() const
{
	if( cls != TypeClassification::function )
		return std::optional<TypeInstance>();

	return this->return_type;
}

Type const*
Type::get_dependent_type() const
{
	// TODO: I'm not sure this is the best way. Perhaps this should return null?
	if( this->dependent_on_type_ )
		return this->dependent_on_type_;
	else
		return this;
}

Type
Type::Function(
	String const& name, Vec<MemberTypeInstance> args, TypeInstance return_type, bool is_var_arg)
{
	return Type{name, args, return_type, is_var_arg};
}

Type
Type::Function(String const& name, Vec<MemberTypeInstance> args, TypeInstance return_type)
{
	return Type{name, args, return_type, false};
}

Type
Type::Struct(String const& name, std::map<String, MemberTypeInstance> members)
{
	return Type{name, members, TypeClassification::struct_cls};
}

Type
Type::Struct(String const& name, std::map<String, MemberTypeInstance> members, EnumNominal nominal)
{
	auto type = Type{name, members, TypeClassification::struct_cls};
	type.nominal_ = nominal;
	return type;
}

Type
Type::Union(String const& name, std::map<String, MemberTypeInstance> members)
{
	return Type{name, members, TypeClassification::union_cls};
}

Type
Type::EnumPartial(String const& name)
{
	return Type{name, {}, TypeClassification::enum_cls};
}

Type
Type::Primitive(String name)
{
	return Type{name};
}

Type
Type::Primitive(String name, int bit_width)
{
	auto type = Type{name};
	type.int_width_ = bit_width;
	return type;
}

Type
Type::Primitive(String name, EnumNominal nominal)
{
	auto type = Type{name};
	type.nominal_ = nominal;
	return type;
}