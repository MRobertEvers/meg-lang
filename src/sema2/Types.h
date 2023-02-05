
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
	Type const* infer_type_;
	Type const* void_type_;
	Type const* i32_type_;
	Type const* i16_type_;
	Type const* i8_type_;
	Type const* u32_type_;
	Type const* u16_type_;
	Type const* u8_type_;

public:
	std::map<String, Type> types;
	Types();

	Type const* define_type(Type type);

	Type const* infer_type();
	Type const* void_type();
	Type const* i32_type();
	Type const* i16_type();
	Type const* i8_type();
	Type const* u32_type();
	Type const* u16_type();
	Type const* u8_type();

	bool equal_types(TypeInstance l, TypeInstance r);
	TypeInstance non_inferred(TypeInstance l, TypeInstance r);

	TypeInstance VoidType();
	TypeInstance InferType();

private:
};

String to_string(TypeInstance ty);

} // namespace sema