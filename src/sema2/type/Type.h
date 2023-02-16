#pragma once
#include "../TypeInstance.h"
#include "../TypedMember.h"
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
		primitive,
	};

	TypeClassification cls = TypeClassification::primitive;
	std::map<String, TypedMember> members;

	Vec<TypedMember> members_order;

	// For functions return type
	TypeInstance return_type;
	bool is_var_arg_;

	String name;

	Type(String name);
	Type(String name, std::map<String, TypedMember> members);
	Type(String name, Vec<TypedMember> args, TypeInstance return_type, bool is_var_arg);

public:
	~Type();

	std::optional<TypeInstance> get_return_type() const;
	bool is_function_type() const { return cls == TypeClassification::function; }
	bool is_struct_type() const { return cls == TypeClassification::struct_cls; }
	String get_name() const;

	bool is_var_arg() const { return is_var_arg_; }

	std::optional<TypedMember> get_member(String const& name) const;
	TypedMember get_member(int idx) const;
	int get_member_count() const;

	static Type Function(String const&, Vec<TypedMember>, TypeInstance, bool);
	static Type Function(String const&, Vec<TypedMember>, TypeInstance);
	static Type Struct(String const& name, std::map<String, TypedMember> members);
	static Type Primitive(String name);
};

} // namespace sema