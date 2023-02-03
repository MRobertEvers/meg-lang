
#pragma once

#include "TypeInstance.h"
#include "TypedMember.h"
#include "common/String.h"
#include "type/Type.h"

#include <map>

namespace sema
{

class Types
{
private:
	Type const* void_type;

public:
	std::map<String, Type> types;
	Types();

	Type const* define_type(Type type);

private:
};

} // namespace sema