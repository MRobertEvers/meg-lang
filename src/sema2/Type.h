#pragma once
#include "common/String.h"
#include "common/Vec.h"

#include <map>
namespace sema
{

static char const infer_type_name[] = "@_infer";

class Type;

class TypeInstance
{
private:
	TypeInstance(Type const* type, int indir)
		: type(type)
		, indirection_level(indir){};

public:
	Type const* type;

	int indirection_level;

	TypeInstance()
		: type(nullptr)
		, indirection_level(0){};

	bool operator==(const TypeInstance& rhs)
	{
		return this->indirection_level == rhs.indirection_level && this->type == rhs.type;
	}
	bool operator!=(const TypeInstance& rhs) { return !operator==(rhs); }

	static TypeInstance OfType(Type const* type) { return TypeInstance(type, 0); }
	static TypeInstance PointerTo(Type const* type, int indirection)
	{
		return TypeInstance(type, indirection);
	}
};

struct TypedMember
{
	TypeInstance type;
	String name;

	TypedMember() = default;
	TypedMember(TypeInstance type, String name)
		: type(type)
		, name(name){};
};

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

	// For structs, this is the members
	// For functions, this is the arguments
	std::map<String, TypedMember> members;

	// Used for functions. TODO: this is ugly.
	Vec<TypedMember> members_order;

	// For functions return type
	TypeInstance return_type;

	String name;

	Type(String name)
		: name(name)
		, cls(TypeClassification::primitive){};

	Type(String name, std::map<String, TypedMember> members)
		: name(name)
		, members(members)
		, cls(TypeClassification::struct_cls){};

	Type(String name, Vec<TypedMember> args, TypeInstance return_type)
		: name(name)
		, return_type(return_type)
		, cls(TypeClassification::function)
	{
		for( auto arg : args )
		{
			members.emplace(arg.name, arg);
		}
	};

public:
	TypeClassification cls = TypeClassification::primitive;

	bool is_infer_type() const { return name == infer_type_name; }
	bool is_function_type() const { return cls == TypeClassification::function; }
	bool is_struct_type() const { return cls == TypeClassification::struct_cls; }
	String get_name() const;

	TypeInstance const* get_member_type(String const& name) const;
	TypeInstance const* get_member_type(int idx) const;
	TypeInstance get_return_type() const;
	void add_member(String const& name, TypeInstance type);

	static Type Function(String const& name, Vec<TypedMember> args, TypeInstance return_type);
	static Type Struct(String const& name, std::map<String, TypedMember> members);
	static Type Primitive(String name);
};

static Type void_type = Type::Primitive("void");
static Type infer_type = Type::Primitive(infer_type_name);
static Type i8_type = Type::Primitive("i8");
static Type i16_type = Type::Primitive("i16");
static Type i32_type = Type::Primitive("i32");
static Type u8_type = Type::Primitive("u8");
static Type u16_type = Type::Primitive("u16");
static Type u32_type = Type::Primitive("u32");
static Type bool_type = Type::Primitive("bool");

} // namespace sema