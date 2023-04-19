#pragma once

#include "QualifiedTy.h"

#include <map>
#include <string>
#include <vector>

enum class TyKind
{
	Invalid,
	Int,
	Bool,
	Void,
	Float,
	Interface,
	Func,
	Enum,
	Struct,
	Union,

	// Frame type is generated from functions
	// annoted with the 'async' keyword.
	// Frames are "impl generator<Iter, Send, Ret>"
	Frame,
};

class Ty;

struct TyInt
{
	static constexpr TyKind tk = TyKind::Int;

	// These are ordered in this way so we can
	// do simple comparisons to for safe conversions
	enum class IntKind : int
	{
		// iX is used for int literals
		// iX is usually coerced to a concrety type on first use.
		iX,
		i8,
		u8,
		i16,
		u16,
		i32,
		u32,
		i64,
		u64,
		i128,
		u128,
	} kind = IntKind::i32;

	enum class Sign
	{
		Any,
		Signed,
		Unsigned
	} sign = Sign::Signed;

	std::string name;

	int width;

	TyInt(std::string name, IntKind kind);
};

struct TyBool
{
	static constexpr TyKind tk = TyKind::Bool;
};

struct TyVoid
{
	static constexpr TyKind tk = TyKind::Void;
};

struct TyFloat
{
	static constexpr TyKind tk = TyKind::Float;

	std::string name;

	int width = 0;

	TyFloat(std::string name, int width)
		: name(name)
		, width(width){};
};

struct TyInterface
{
	static constexpr TyKind tk = TyKind::Interface;
	std::string name;

	// Function members
	std::map<std::string, QualifiedTy> members;

	TyInterface(std::string name, std::map<std::string, QualifiedTy> members)
		: name(name)
		, members(members){};
};

struct TyFunc
{
	static constexpr TyKind tk = TyKind::Func;
	std::string name;

	std::vector<QualifiedTy> args_qtys;
	QualifiedTy rt_qty;
	bool is_var_arg;

	TyFunc(
		std::string name, std::vector<QualifiedTy> args_qtys, QualifiedTy rt_qty, bool is_var_arg)
		: name(name)
		, args_qtys(args_qtys)
		, is_var_arg(is_var_arg)
		, rt_qty(rt_qty){};
};

struct TyStruct
{
	static constexpr TyKind tk = TyKind::Struct;

	// Pointers to interfaces implemented by this struct.
	std::vector<Ty const*> implements;

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

struct TyEnum
{
	static constexpr TyKind tk = TyKind::Enum;
	std::string name;

	TyEnum(std::string name)
		: name(name){};
};

struct TyFrame
{
	static constexpr TyKind tk = TyKind::Frame;
	std::string name;

	TyFrame(std::string name)
		: name(name){};
};

struct Ty
{
	TyKind kind = TyKind::Invalid;
	union TyData
	{
		TyInt ty_int;
		TyBool ty_bool;
		TyVoid ty_void;
		TyFloat ty_float;
		TyFunc ty_func;
		TyStruct ty_struct;
		TyUnion ty_union;
		TyEnum ty_enum;
		TyFrame ty_frame;
		TyInterface ty_interface;

		// Attention! This leaks!
		TyData() {}
		// TODO: Deleter
		~TyData() {}
	} data; //
};

template<typename Node, typename TyTy>
auto&
ty_cast(TyTy* ty)
{
	assert(ty->kind == Node::tk && "Invalid ty_cast");
	if constexpr( std::is_same_v<TyInt, Node> )
		return ty->data.ty_int;
	else if constexpr( std::is_same_v<TyFunc, Node> )
		return ty->data.ty_func;
	else if constexpr( std::is_same_v<TyStruct, Node> )
		return ty->data.ty_struct;
	else if constexpr( std::is_same_v<TyUnion, Node> )
		return ty->data.ty_union;
	else if constexpr( std::is_same_v<TyEnum, Node> )
		return ty->data.ty_enum;
	else if constexpr( std::is_same_v<TyBool, Node> )
		return ty->data.ty_bool;
	else if constexpr( std::is_same_v<TyVoid, Node> )
		return ty->data.ty_void;
	else if constexpr( std::is_same_v<TyFloat, Node> )
		return ty->data.ty_float;
	else if constexpr( std::is_same_v<TyInterface, Node> )
		return ty->data.ty_interface;
	else
		static_assert("Bad ty cast");
}