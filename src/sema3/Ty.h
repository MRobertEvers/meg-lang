#pragma once
#include <string>

enum class TyKind
{
	Invalid,
	Primitive,
	Func,
	Enum,
	Struct,
};

struct TyPrimitive
{
	static constexpr TyKind tk = TyKind::Primitive;
	std::string name;

	TyPrimitive(std::string name)
		: name(name){};
};

struct TyFunc
{
	static constexpr TyKind tk = TyKind::Func;
	std::string name;

	TyFunc(std::string name)
		: name(name){};
};

struct Ty
{
	TyKind kind = TyKind::Invalid;
	union TyData
	{
		TyPrimitive ty_primitive;
		TyFunc ty_func;

		// Attention! This leaks!
		TyData() {}
		// TODO: Deleter
		~TyData() {}
	} data; //
};