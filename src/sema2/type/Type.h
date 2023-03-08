#pragma once
#include "../MemberTypeInstance.h"
#include "../TypeInstance.h"
#include "EnumNominal.h"
#include "FunctionTypeInfo.h"
#include "StructTypeInfo.h"

#include <map>
#include <optional>
#include <string>
#include <vector>
namespace sema
{

/**
 * @brief Contains information about type names.
 *
 * Type names can be Struct names or Function names,
 * and, in the future, anything else that can be considered
 * a type.
 *
 * Provides helper methods for looking up namespaced members
 * or arguments for functions.
 *
 */
class Type
{
private:
	enum class Kind
	{
		None,
		Function,
		Struct,
		Union,
		Enum,
		Primitive,
		Generic,
		Template,
	};

	Kind cls = Kind::Primitive;
	Kind template_cls = Kind::None;
	std::map<std::string, MemberTypeInstance> members;

	std::vector<MemberTypeInstance> members_order;
	std::vector<TypeInstance> type_parameters;

	// For integers
	int int_width_ = 0;

	// For functions return type
	TypeInstance return_type;
	bool is_var_arg_;

	// For enums
	Type const* dependent_on_type_ = nullptr;
	std::optional<EnumNominal> nominal_;

	// All Type classes
	std::string name;

	Type(std::string name);
	Type(std::string name, std::vector<TypeInstance> type_parameters, Kind, Kind);
	Type(std::string name, std::map<std::string, MemberTypeInstance> members, Kind);
	Type(
		std::string name,
		std::vector<MemberTypeInstance> args,
		TypeInstance return_type,
		bool is_var_arg);

public:
	// Impl
	bool is_impl_;

	~Type();

	std::optional<TypeInstance> get_return_type() const;
	bool is_function_type() const { return cls == Kind::Function; }
	bool is_struct_type() const { return cls == Kind::Struct; }
	bool is_union_type() const { return cls == Kind::Union; }
	bool is_enum_type() const { return cls == Kind::Enum; }
	bool is_template() const { return cls == Kind::Template; }
	std::string get_name() const;

	// Enum members only
	EnumNominal as_nominal() const;

	// Integer types
	int int_width() const { return int_width_; }

	// TODO: Assert we are a function.
	bool is_var_arg() const { return is_var_arg_; }

	std::optional<MemberTypeInstance> get_member(std::string const& name) const;
	MemberTypeInstance get_member(int idx) const;
	int get_member_count() const;

	// The enum members depend on the parent type.
	void set_enum_members(std::map<std::string, MemberTypeInstance> members);
	Type const* get_dependent_type() const;
	void set_dependent_type(Type const* t) { this->dependent_on_type_ = t; };

	Type instantiate_template(std::vector<TypeInstance> concrete_types) const;

	static Type Function(std::string const&, std::vector<MemberTypeInstance>, TypeInstance, bool);
	static Type Function(std::string const&, std::vector<MemberTypeInstance>, TypeInstance);
	static Type Struct(std::string const& name, std::map<std::string, MemberTypeInstance> members);
	static Type
	Struct(std::string const& name, std::map<std::string, MemberTypeInstance>, EnumNominal);
	static Type Union(std::string const& name, std::map<std::string, MemberTypeInstance> members);
	static Type EnumPartial(std::string const& name);
	static Type Generic(std::string name);
	static Type Template(std::string name, std::vector<TypeInstance> type_params);
	static Type Primitive(std::string name);
	static Type Primitive(std::string name, int bit_width);
	static Type Primitive(std::string name, EnumNominal);
};

} // namespace sema