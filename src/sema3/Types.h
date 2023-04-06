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

template<typename Type, typename... Args>
Ty const*
Types::create(Args&&... args)
{
	Ty* ty = &types.emplace_back();
	ty->kind = Type::tk;

	if constexpr( std::is_same_v<TyFunc, Type> )
		ty->data.ty_func = Type(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<TyPrimitive, Type> )
		ty->data.ty_primitive = Type(std::forward<Args>(args)...);
	else
		static_assert("Cannot create hir node of type " + to_string(Type::tk));

	return ty;
}