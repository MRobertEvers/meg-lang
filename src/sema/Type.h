#pragma once
#include "common/String.h"
#include "common/Vec.h"

#include <map>
namespace sema
{

static char const* infer_type_name = "@_infer";

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
		pointer
	};

	// For structs, this is the members
	// For functions, this is the arguments
	std::map<String, Type const*> members;

	// For functions return type
	Type const* return_type = nullptr;

	// For pointer
	Type const* base = nullptr;

	// For all types except pointer
	String name;

	Type(Type const& base, bool dummy)
		: base(&base)
		, cls(TypeClassification::pointer){};

	Type(String name)
		: name(name)
		, cls(TypeClassification::primitive){};

	Type(String name, std::map<String, Type const*> members)
		: name(name)
		, members(members)
		, cls(TypeClassification::struct_cls){};

	Type(String name, Vec<Type const*> args, Type const* return_type)
		: name(name)
		, return_type(return_type)
		, cls(TypeClassification::function)
	{
		for( auto arg : args )
		{
			members.emplace(arg->get_name(), arg);
		}
	};

public:
	TypeClassification cls = TypeClassification::primitive;

	// Copy constructor
	Type(Type const& other)
		: base(other.base)
		, name(other.name)
		, members(other.members)
		, return_type(other.return_type)
		, cls(other.cls){};

	Type(Type&& other)
		: base(std::move(other.base))
		, name(std::move(other.name))
		, members(std::move(other.members))
		, return_type(other.return_type)
		, cls(other.cls)
	{
		other.base = nullptr;
	};

	bool is_infer_type() const { return name == infer_type_name; }
	bool is_pointer_type() const { return cls == TypeClassification::pointer; }
	String get_name() const;

	Type const* get_member_type(String const& name) const;
	void add_member(String const& name, Type const& type);

	static Type PointerTo(Type const& base);
	static Type Function(String const& name, Vec<Type const*> args, Type const* return_type);
	static Type Struct(String const& name, std::map<String, Type const*> members);
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

} // namespace sema