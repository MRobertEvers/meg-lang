#pragma once

namespace sema
{
class Type;
class TypeInstance
{
private:
	TypeInstance(Type const* type, int indir)
		: type(type)
		, indirection_level(indir)
		, is_array_(false)
		, array_size(0){};

	TypeInstance(Type const* type, int indir, int array_size)
		: type(type)
		, indirection_level(indir)
		, is_array_(true)
		, array_size(array_size){};

	bool is_array_ = false;

public:
	TypeInstance() = default;
	int array_size;
	int indirection_level;
	Type const* type;
	bool operator==(const TypeInstance& rhs)
	{
		return this->indirection_level == rhs.indirection_level && this->type == rhs.type;
	}
	bool operator!=(const TypeInstance& rhs) { return !operator==(rhs); }

	bool is_function_type() const;
	bool is_struct_type() const;
	bool is_pointer_type() const { return !is_array_ && indirection_level != 0; }
	bool is_array_type() const { return is_array_; }

	TypeInstance PointerTo(int indirection);
	TypeInstance PointerElementType();
	TypeInstance ArrayElementType();

	static TypeInstance OfType(Type const* type);
	static TypeInstance PointerTo(Type const* type, int indirection);
	static TypeInstance ArrayOf(TypeInstance type, int array_size);
};

}; // namespace sema