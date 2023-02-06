#pragma once
#include "TypeInstance.h"
#include "common/String.h"

namespace sema
{
struct TypedMember
{
	TypeInstance type;
	String name;
	int idx;

	TypedMember(TypeInstance type, String name, int idx)
		: type(type)
		, name(name)
		, idx(idx){};
};
}; // namespace sema