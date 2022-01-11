#pragma once
#include "common/String.h"

#include <map>
namespace ast
{
class Type
{
private:
	std::map<String, Type const*> members;

	Type(Type const& base, bool dummy)
		: base(&base){};

public:
	// TODO: Cant have both base and members
	Type const* base = nullptr;

	String name;
	// explicit Type(String name)
	// 	: name(name){};
	// explicit Type(String& name)
	// 	: name(name){};
	explicit Type(String&& name)
		: name(name){};
	explicit Type(String const& name)
		: name(name){};
	explicit Type(char const* name)
		: name(name){};
	// Copy constructor
	Type(Type const& other)
		: base(other.base)
		, name(other.name)
		, members(other.members){};

	Type(Type&& other)
		: base(std::move(other.base))
		, name(std::move(other.name))
		, members(std::move(other.members))
	{
		other.base = nullptr;
	};

	// ~Type()
	// {
	// 	if( base != nullptr )
	// 	{
	// 		delete base;
	// 	}
	// }

	bool is_pointer_type() const { return base != nullptr; }

	Type const* get_member_type(String const& name) const;
	void add_member(String const& name, Type const& type);

	static Type PointerTo(Type const& base) { return Type(base, true); }
};

static Type void_type{"void"};
static Type infer_type{"@_infer"};
static Type i8_type{"i8"};
static Type i16_type{"i16"};
static Type i32_type{"i32"};
static Type u8_type{"u8"};
static Type u16_type{"u16"};
static Type u32_type{"u32"};

} // namespace ast