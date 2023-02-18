

#pragma once

namespace cg
{

class LLVMType
{
	enum class Type
	{
		Union,
		Primitive,
		Struct,
		Function,
		Enum,
	};
	Type type = Type::Primitive;
};
} // namespace cg