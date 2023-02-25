
#pragma once

#include "Type.h"
#include "TypeInstance.h"

#include <map>
#include <string>

namespace sema
{

class Types
{
private:
	// Builtin Types.
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

	// TODO: Need stable storage.
	std::vector<Type> types_;

public:
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

std::string to_string(TypeInstance ty);

} // namespace sema