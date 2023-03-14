

#include "Type.h"

using namespace sema;

Type::Type(std::string name)
	: name(name)
	, is_var_arg_(false)
	, cls(Kind::Primitive){};

Type::Type(std::string name, std::vector<TypeInstance> type_parameters, Kind cls, Kind template_cls)
	: name(name)
	, type_parameters(type_parameters)
	, template_cls(template_cls)
	, cls(cls){};

Type::Type(std::string name, std::map<std::string, MemberTypeInstance> members, Kind cls)
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

Type::Type(
	std::string name,
	std::vector<MemberTypeInstance> args,
	TypeInstance return_type,
	bool is_var_arg)
	: name(name)
	, members_order(args)
	, return_type(return_type)
	, is_var_arg_(is_var_arg)
	, cls(Kind::Function)
{
	for( auto arg : args )
	{
		// members.emplace(arg.name, arg);
	}
};

Type::~Type()
{
	// switch( cls )
	// {
	// case Kind::Function:
	// 	this->type_info.fn_.~FunctionTypeInfo();
	// 	break;
	// case Kind::Struct:
	// 	this->type_info.struct_.~StructTypeInfo();
	// 	break;
	// case Kind::Primitive:
	// 	break;
	// }
}

std::string
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
Type::get_member(std::string const& name) const
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
	if( cls == Kind::Function )
		return members_order.size();
	else
		return members.size();
}

TypeInstance
Type::get_type_parameter(int idx) const
{
	assert(idx < type_parameters.size());

	return type_parameters.at(idx);
}

int
Type::get_type_parameter_count() const
{
	return type_parameters.size();
}

void
Type::set_enum_members(std::map<std::string, MemberTypeInstance> members)
{
	this->members = members;
	for( auto member : members )
		members_order.push_back(member.second);
}

std::optional<TypeInstance>
Type::get_return_type() const
{
	if( cls != Kind::Function )
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
Type::instantiate_template(std::vector<TypeInstance> concrete_types) const
{
	assert(concrete_types.size() == type_parameters.size());

	// TODO: template name
	return Type("template", concrete_types, template_cls, Kind::None);
}

Type
Type::Function(
	std::string const& name,
	std::vector<MemberTypeInstance> args,
	TypeInstance return_type,
	bool is_var_arg)
{
	return Type{name, args, return_type, is_var_arg};
}

Type
Type::Function(
	std::string const& name, std::vector<MemberTypeInstance> args, TypeInstance return_type)
{
	return Type{name, args, return_type, false};
}

Type
Type::Struct(std::string const& name, std::map<std::string, MemberTypeInstance> members)
{
	return Type{name, members, Kind::Struct};
}

Type
Type::Struct(
	std::string const& name, std::map<std::string, MemberTypeInstance> members, EnumNominal nominal)
{
	auto type = Type{name, members, Kind::Struct};
	type.nominal_ = nominal;
	return type;
}

Type
Type::Union(std::string const& name, std::map<std::string, MemberTypeInstance> members)
{
	return Type{name, members, Kind::Union};
}

Type
Type::EnumPartial(std::string const& name)
{
	return Type{name, {}, Kind::Enum};
}

Type
Type::Generic(std::string name)
{
	return Type{name, {}, Kind::Generic};
}

Type
Type::Template(std::string name, std::vector<TypeInstance> type_params)
{
	//
	return Type{name, type_params, Kind::Template, Kind::Struct};
}

Type
Type::Primitive(std::string name)
{
	return Type{name};
}

Type
Type::Primitive(std::string name, int bit_width)
{
	auto type = Type{name};
	type.int_width_ = bit_width;
	return type;
}

Type
Type::Primitive(std::string name, EnumNominal nominal)
{
	auto type = Type{name};
	type.nominal_ = nominal;
	return type;
}