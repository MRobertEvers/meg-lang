#pragma once
#include "../MemberTypeInstance.h"
#include "../TypeInstance.h"
#include "common/String.h"
#include "common/Vec.h"

#include <map>

namespace sema
{
struct FunctionTypeInfo
{
	// For structs, this is the members
	// For functions, this is the arguments
	std::map<String, MemberTypeInstance> members;

	Vec<MemberTypeInstance> members_order;

	// For functions return type
	TypeInstance return_type;

	// static FunctionTypeInfo Make(Vec<TypedMember> args, TypeInstance return_type);
};
} // namespace sema