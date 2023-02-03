

#include "Types.h"

using namespace sema;

static char const infer_type_name[] = "@_infer";

static Type _void_type = Type::Primitive("void");
static Type _infer_type = Type::Primitive(infer_type_name);
static Type _i8_type = Type::Primitive("i8");
static Type _i16_type = Type::Primitive("i16");
static Type _i32_type = Type::Primitive("i32");
static Type _u8_type = Type::Primitive("u8");
static Type _u16_type = Type::Primitive("u16");
static Type _u32_type = Type::Primitive("u32");
static Type _bool_type = Type::Primitive("bool");

Types::Types()
{
	this->void_type_ = define_type(_void_type);
	this->i32_type_ = define_type(_infer_type);
	define_type(_i8_type);
	define_type(_i16_type);
	define_type(_i32_type);
	define_type(_u8_type);
	define_type(_u16_type);
	define_type(_u32_type);
	define_type(_bool_type);
}

Type const*
Types::define_type(Type type)
{
	auto emplaced = types.emplace(type.get_name(), type);

	return &emplaced.first->second;
}

Type const*
Types::void_type()
{
	return this->void_type_;
}

Type const*
Types::i32_type()
{
	return this->i32_type_;
}
