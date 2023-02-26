

#include "Types.h"

using namespace ir;

static char const infer_type_name[] = "@_infer";

static Type _void_type = Type::Primitive("void");
static Type _infer_type = Type::Primitive(infer_type_name);
static Type _i8_type = Type::Primitive("i8", 8);
static Type _i16_type = Type::Primitive("i16", 16);
static Type _i32_type = Type::Primitive("i32", 32);
static Type _i64_type = Type::Primitive("i64", 64);
static Type _u8_type = Type::Primitive("u8", 8);
static Type _u16_type = Type::Primitive("u16", 16);
static Type _u32_type = Type::Primitive("u32", 32);
static Type _u64_type = Type::Primitive("u64", 64);
static Type _bool_type = Type::Primitive("bool");

Types::Types()
{
	types_.reserve(1000);
	this->void_type_ = define_type(_void_type);
	this->infer_type_ = define_type(_infer_type);
	this->i8_type_ = define_type(_i8_type);
	this->i16_type_ = define_type(_i16_type);
	this->i32_type_ = define_type(_i32_type);
	this->i64_type_ = define_type(_i64_type);
	this->u8_type_ = define_type(_u8_type);
	this->u16_type_ = define_type(_u16_type);
	this->u32_type_ = define_type(_u32_type);
	this->u64_type_ = define_type(_u64_type);
	this->bool_type_ = define_type(_bool_type);
}

Type*
Types::define_type(Type type)
{
	auto& emplaced = types_.emplace_back(type);

	return &emplaced;
}

Type const*
Types::infer_type()
{
	return this->infer_type_;
}

Type const*
Types::void_type()
{
	return this->void_type_;
}

Type const*
Types::i64_type()
{
	return this->i64_type_;
}

Type const*
Types::i32_type()
{
	return this->i32_type_;
}

Type const*
Types::i16_type()
{
	return this->i16_type_;
}

Type const*
Types::i8_type()
{
	return this->i8_type_;
}

Type const*
Types::u64_type()
{
	return this->u64_type_;
}

Type const*
Types::u32_type()
{
	return this->u32_type_;
}

Type const*
Types::u16_type()
{
	return this->u16_type_;
}

Type const*
Types::u8_type()
{
	return this->u8_type_;
}

Type const*
Types::bool_type()
{
	return this->bool_type_;
}

bool
Types::equal_types(TypeInstance l, TypeInstance r)
{
	if( l.type == infer_type_ || r.type == infer_type_ )
		return l.type != r.type;

	return l == r;
}

bool
Types::is_infer_type(TypeInstance l)
{
	return l.type == infer_type_;
}

bool
Types::is_integer_type(TypeInstance l)
{
	if( l.is_array_type() || l.is_struct_type() || l.is_function_type() )
		return false;

	return l.type == i8_type_ || l.type == i16_type_ || l.type == i32_type_ ||
		   l.type == i64_type_ || l.type == u8_type_ || l.type == u16_type_ ||
		   l.type == u32_type_ || l.type == u64_type_;
}

bool
Types::is_signed_integer_type(TypeInstance l)
{
	if( l.is_array_type() || l.is_struct_type() || l.is_function_type() )
		return false;

	return l.type == i8_type_ || l.type == i16_type_ || l.type == i32_type_ || l.type == i64_type_;
}

bool
Types::is_unsigned_integer_type(TypeInstance l)
{
	if( l.is_array_type() || l.is_struct_type() || l.is_function_type() )
		return false;

	return l.type == u8_type_ || l.type == u16_type_ || l.type == u32_type_ || l.type == u64_type_;
}

TypeInstance
Types::non_inferred(TypeInstance l, TypeInstance r)
{
	if( l.type != infer_type_ )
		return l;
	if( r.type != infer_type_ )
		return r;

	assert(false);
}

TypeInstance
Types::BoolType()
{
	return TypeInstance::OfType(bool_type_);
}

TypeInstance
Types::VoidType()
{
	return TypeInstance::OfType(void_type_);
}

TypeInstance
Types::InferType()
{
	return TypeInstance::OfType(infer_type_);
}

std::string
ir::to_string(TypeInstance ty)
{
	return ty.type->get_name() + std::string(ty.indirection_level, '*');
}