#pragma once
#include "TypeInstance.h"

#include <string>

namespace ir
{
struct MemberTypeInstance
{
	TypeInstance type;
	std::string name;

	/**
	 * @brief Order index of member in struct.
	 *
	 * Useful in codegen for lookin up members.
	 */
	int idx;

	MemberTypeInstance() = default;
	MemberTypeInstance(TypeInstance type, std::string name, int idx)
		: type(type)
		, name(name)
		, idx(idx){};
};
}; // namespace ir