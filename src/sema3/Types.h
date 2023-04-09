#pragma once
#include "Ty.h"

#include <deque>

class Types
{
	std::deque<Ty> types;

public:
	Types();

	template<typename Type, typename... Args>
	Ty const* create(Args&&... args);
};

template<typename Node, typename TyTy>
auto&
ty_cast(TyTy* ty)
{
	assert(ty->kind == Node::tk && "Invalid ty_cast");
	if constexpr( std::is_same_v<TyPrimitive, Node> )
		return ty->data.ty_primitive;
	else if constexpr( std::is_same_v<TyFunc, Node> )
		return ty->data.ty_func;
	else if constexpr( std::is_same_v<TyStruct, Node> )
		return ty->data.ty_struct;
	else if constexpr( std::is_same_v<TyUnion, Node> )
		return ty->data.ty_union;
	else if constexpr( std::is_same_v<TyEnum, Node> )
		return ty->data.ty_enum;
	else
		static_assert("Bad ty cast");
}

template<typename Type, typename... Args>
Ty const*
Types::create(Args&&... args)
{
	Ty* ty = &types.emplace_back();
	ty->kind = Type::tk;

	ty_cast<Type>(ty) = Type(std::forward<Args>(args)...);

	return ty;
}