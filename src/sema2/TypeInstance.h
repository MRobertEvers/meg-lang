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
	TypeInstance() = default;

	int indirection_level;
	Type const* type;
	bool operator==(const TypeInstance& rhs)
	{
		return this->indirection_level == rhs.indirection_level && this->type == rhs.type;
	}
	bool operator!=(const TypeInstance& rhs) { return !operator==(rhs); }

	bool is_function_type() const;
	bool is_struct_type() const;
	bool is_pointer_type() const { return indirection_level != 0; }

	static TypeInstance OfType(Type const* type);
	static TypeInstance PointerTo(Type const* type, int indirection);
};

}; // namespace sema