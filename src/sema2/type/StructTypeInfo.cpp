#include "StructTypeInfo.h"

using namespace sema;

StructTypeInfo::StructTypeInfo(std::map<String, TypedMember> members)
	: members(members)
{}