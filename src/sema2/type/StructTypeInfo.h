#pragma once
#include "../TypeInstance.h"
#include "../TypedMember.h"
#include "common/String.h"
#include "common/Vec.h"

#include <map>

namespace sema
{

struct StructTypeInfo
{
	std::map<String, TypedMember> members;

public:
	// static StructTypeInfo Make(std::map<String, TypedMember> members);
};

} // namespace sema