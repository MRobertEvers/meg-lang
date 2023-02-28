#pragma once
#include "TypeInstance.h"

#include <string>

namespace sema
{
struct MemberTypeInstance
{
	TypeInstance type;

	/**
	 * @brief Order index of member in struct.
	 *
	 * Useful in codegen for lookin up members.
	 */
	int idx;

	MemberTypeInstance() = default;
	MemberTypeInstance(TypeInstance type, int idx)
		: type(type)
		, idx(idx){};
};
}; // namespace sema