
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
	Type const* void_type_;
	Type const* i32_type_;

public:
	std::map<String, Type> types;
	Types();

	Type const* define_type(Type type);

	Type const* void_type();
	Type const* i32_type();

private:
};

} // namespace sema