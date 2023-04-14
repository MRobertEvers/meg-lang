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

	ty_cast<Type>(ty) = Type(std::forward<Args>(args)...);

	return ty;
}