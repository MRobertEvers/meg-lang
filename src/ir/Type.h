#pragma once
#include "EnumNominal.h"
#include "MemberTypeInstance.h"
#include "TypeInstance.h"

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
		Function,
		Struct,
		Union,
		Enum,
		Primitive,
	};

	Kind kind = Kind::Primitive;
	std::map<std::string, MemberTypeInstance> members;

	std::vector<MemberTypeInstance> members_order;

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
	Type(std::string name, std::map<std::string, MemberTypeInstance> members, Kind);
	Type(
		std::string name,
		std::vector<MemberTypeInstance> args,
		TypeInstance return_type,
		bool is_var_arg);

public:
	~Type();

	std::optional<TypeInstance> get_return_type() const;
	bool is_function_type() const { return kind == Kind::Function; }
	bool is_struct_type() const { return kind == Kind::Struct; }
	bool is_union_type() const { return kind == Kind::Union; }
	bool is_enum_type() const { return kind == Kind::Enum; }
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

	static Type Function(std::string const&, std::vector<MemberTypeInstance>, TypeInstance, bool);
	static Type Function(std::string const&, std::vector<MemberTypeInstance>, TypeInstance);
	static Type Struct(std::string const& name, std::map<std::string, MemberTypeInstance> members);
	static Type
	Struct(std::string const& name, std::map<std::string, MemberTypeInstance>, EnumNominal);
	static Type Union(std::string const& name, std::map<std::string, MemberTypeInstance> members);
	static Type EnumPartial(std::string const& name);
	static Type Primitive(std::string name);
	static Type Primitive(std::string name, int bit_width);
	static Type Primitive(std::string name, EnumNominal);
};

} // namespace sema