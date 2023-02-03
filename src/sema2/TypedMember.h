#pragma once
#include "TypeInstance.h"
#include "common/String.h"

namespace sema
{
struct TypedMember
{
	TypeInstance type;
	String name;

	TypedMember(TypeInstance type, String name)
		: type(type)
		, name(name){};
};
}; // namespace sema