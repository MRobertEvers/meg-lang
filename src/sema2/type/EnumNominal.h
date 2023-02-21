#pragma once

namespace sema
{
/**
 * @brief Holds the 'nominal' value of the enum member.
 */
struct EnumNominal
{
	long long value;

	EnumNominal() = default;
	EnumNominal(long long value)
		: value(value)
	{}
};
} // namespace sema