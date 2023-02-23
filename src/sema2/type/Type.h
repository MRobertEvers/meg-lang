#pragma once
#include "../MemberTypeInstance.h"
#include "../TypeInstance.h"
#include "EnumNominal.h"
#include "FunctionTypeInfo.h"
#include "StructTypeInfo.h"
#include "common/String.h"
#include "common/Vec.h"

#include <map>
#include <optional>
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
	enum class TypeClassification
	{
		function,
		struct_cls,
		union_cls,
		enum_cls,
		enum_member_cls,
		primitive,
	};

	TypeClassification cls = TypeClassification::primitive;
	std::map<String, MemberTypeInstance> members;

	Vec<MemberTypeInstance> members_order;

	// For integers
	int int_width_ = 0;

	// For functions return type
	TypeInstance return_type;
	bool is_var_arg_;

	// For enums
	Type const* dependent_on_type_ = nullptr;
	std::optional<EnumNominal> nominal_;

	// All Type classes
	String name;

	Type(String name);
	Type(String name, std::map<String, MemberTypeInstance> members, TypeClassification);
	Type(String name, Vec<MemberTypeInstance> args, TypeInstance return_type, bool is_var_arg);

public:
	~Type();

	std::optional<TypeInstance> get_return_type() const;
	bool is_function_type() const { return cls == TypeClassification::function; }
	bool is_struct_type() const { return cls == TypeClassification::struct_cls; }
	bool is_union_type() const { return cls == TypeClassification::union_cls; }
	bool is_enum_type() const { return cls == TypeClassification::enum_cls; }
	String get_name() const;

	// Enum members only
	EnumNominal as_nominal() const;

	// Integer types
	int int_width() const { return int_width_; }

	// TODO: Assert we are a function.
	bool is_var_arg() const { return is_var_arg_; }

	std::optional<MemberTypeInstance> get_member(String const& name) const;
	MemberTypeInstance get_member(int idx) const;
	int get_member_count() const;

	// The enum members depend on the parent type.
	void set_enum_members(std::map<String, MemberTypeInstance> members);
	Type const* get_dependent_type() const;
	void set_dependent_type(Type const* t) { this->dependent_on_type_ = t; };

	static Type Function(String const&, Vec<MemberTypeInstance>, TypeInstance, bool);
	static Type Function(String const&, Vec<MemberTypeInstance>, TypeInstance);
	static Type Struct(String const& name, std::map<String, MemberTypeInstance> members);
	static Type Struct(String const& name, std::map<String, MemberTypeInstance>, EnumNominal);
	static Type Union(String const& name, std::map<String, MemberTypeInstance> members);
	static Type EnumPartial(String const& name);
	static Type Primitive(String name);
	static Type Primitive(String name, int bit_width);
	static Type Primitive(String name, EnumNominal);
};

} // namespace sema