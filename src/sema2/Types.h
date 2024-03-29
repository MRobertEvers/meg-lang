
#pragma once

#include "MemberTypeInstance.h"
#include "TypeInstance.h"
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
	Type const* i64_type_;
	Type const* i32_type_;
	Type const* i16_type_;
	Type const* i8_type_;
	Type const* u64_type_;
	Type const* u32_type_;
	Type const* u16_type_;
	Type const* u8_type_;
	Type const* bool_type_;

public:
	std::map<String, Type> types;
	Types();

	Type* define_type(Type type);

	Type const* infer_type();
	Type const* void_type();
	Type const* i64_type();
	Type const* i32_type();
	Type const* i16_type();
	Type const* i8_type();
	Type const* u64_type();
	Type const* u32_type();
	Type const* u16_type();
	Type const* u8_type();
	Type const* bool_type();

	bool equal_types(TypeInstance l, TypeInstance r);
	bool is_infer_type(TypeInstance l);
	bool is_integer_type(TypeInstance l);
	bool is_signed_integer_type(TypeInstance l);
	bool is_unsigned_integer_type(TypeInstance l);

	TypeInstance non_inferred(TypeInstance l, TypeInstance r);

	TypeInstance BoolType();
	TypeInstance VoidType();
	TypeInstance InferType();

private:
};

String to_string(TypeInstance ty);

} // namespace sema