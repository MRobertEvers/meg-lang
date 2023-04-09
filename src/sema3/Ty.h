#pragma once

#include "QualifiedTy.h"

#include <map>
#include <string>
#include <vector>

enum class TyKind
{
	Invalid,
	Primitive,
	Func,
	Enum,
	Struct,
	Union
};

class Ty;

struct TyPrimitive
{
	static constexpr TyKind tk = TyKind::Primitive;
	std::string name;

	int width = 0;

	TyPrimitive(std::string name, int width)
		: name(name)
		, width(width){};
};

struct TyFunc
{
	static constexpr TyKind tk = TyKind::Func;
	std::string name;

	std::vector<QualifiedTy> args_qtys;
	QualifiedTy rt_qty;

	TyFunc(std::string name, std::vector<QualifiedTy> args_qtys, QualifiedTy rt_qty)
		: name(name)
		, args_qtys(args_qtys)
		, rt_qty(rt_qty){};
};

struct TyStruct
{
	static constexpr TyKind tk = TyKind::Struct;
	std::string name;

	std::map<std::string, QualifiedTy> members;

	TyStruct(std::string name, std::map<std::string, QualifiedTy> members)
		: name(name)
		, members(members){};
};

struct TyUnion
{
	static constexpr TyKind tk = TyKind::Union;
	std::string name;

	std::map<std::string, QualifiedTy> members;

	TyUnion(std::string name, std::map<std::string, QualifiedTy> members)
		: name(name)
		, members(members){};
};

struct Ty
{
	TyKind kind = TyKind::Invalid;
	union TyData
	{
		TyPrimitive ty_primitive;
		TyFunc ty_func;
		TyStruct ty_struct;
		TyUnion ty_union;

		// Attention! This leaks!
		TyData() {}
		// TODO: Deleter
		~TyData() {}
	} data; //
};
