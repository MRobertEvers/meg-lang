#pragma once
#include "TypeInstance.h"
#include "common/String.h"

namespace sema
{
struct MemberTypeInstance
{
	TypeInstance type;
	String name;

	/**
	 * @brief Order index of member in struct.
	 *
	 * Useful in codegen for lookin up members.
	 */
	int idx;

	MemberTypeInstance() = default;
	MemberTypeInstance(TypeInstance type, String name, int idx)
		: type(type)
		, name(name)
		, idx(idx){};
};
}; // namespace sema