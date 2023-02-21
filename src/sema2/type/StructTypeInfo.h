#pragma once
#include "../MemberTypeInstance.h"
#include "../TypeInstance.h"
#include "common/String.h"
#include "common/Vec.h"

#include <map>

namespace sema
{

struct StructTypeInfo
{
	std::map<String, MemberTypeInstance> members;

public:
	// static StructTypeInfo Make(std::map<String, TypedMember> members);
};

} // namespace sema