#pragma once

namespace sema
{
class Type;
class TypeInstance
{
private:
	TypeInstance(Type const* type, int indir)
		: type(type)
		, indirection_level(indir){};

public:
	int indirection_level;
	Type const* type;
	bool operator==(const TypeInstance& rhs)
	{
		return this->indirection_level == rhs.indirection_level && this->type == rhs.type;
	}
	bool operator!=(const TypeInstance& rhs) { return !operator==(rhs); }

	static TypeInstance OfType(Type const* type);
	static TypeInstance PointerTo(Type const* type, int indirection);
};

}; // namespace sema