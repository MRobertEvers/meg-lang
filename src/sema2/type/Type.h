#pragma once
#include "../TypeInstance.h"
#include "../TypedMember.h"
#include "FunctionTypeInfo.h"
#include "StructTypeInfo.h"
#include "common/String.h"
#include "common/Vec.h"

#include <map>
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
	union TypeInfo
	{
		FunctionTypeInfo* fn_;
		StructTypeInfo* struct_;
	} type_info;

	String name;

	Type(String name);
	Type(String name, std::map<String, TypedMember> members);
	Type(String name, Vec<TypedMember> args, TypeInstance return_type);

public:
	~Type();

	bool is_function_type() const { return cls == TypeClassification::function; }
	bool is_struct_type() const { return cls == TypeClassification::struct_cls; }
	String get_name() const;

	static Type Function(String const& name, Vec<TypedMember> args, TypeInstance return_type);
	static Type Struct(String const& name, std::map<String, TypedMember> members);
	static Type Primitive(String name);
};

} // namespace sema