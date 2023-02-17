#include "TypeInstance.h"

// TODO: Fix circular deps.
#include "type/Type.h"

using namespace sema;

bool
TypeInstance::is_function_type() const
{
	return !is_array_ && indirection_level == 0 && type->is_function_type();
}

bool
TypeInstance::is_struct_type() const
{
	return !is_array_ && indirection_level == 0 && type->is_struct_type();
}

TypeInstance
TypeInstance::PointerTo(int indirection)
{
	return TypeInstance(this->type, this->indirection_level + indirection);
}

TypeInstance
TypeInstance::PointerElementType()
{
	assert(!is_array_ && this->indirection_level > 0);
	return TypeInstance(this->type, this->indirection_level - 1);
}

TypeInstance
TypeInstance::ArrayElementType()
{
	assert(is_array_);
	return TypeInstance(this->type, this->indirection_level);
}

TypeInstance
TypeInstance::OfType(Type const* type)
{
	return TypeInstance(type, 0);
}

TypeInstance
TypeInstance::PointerTo(Type const* type, int indirection)
{
	return TypeInstance(type, indirection);
}

TypeInstance
TypeInstance::ArrayOf(TypeInstance type, int array_size)
{
	return TypeInstance(type.type, type.indirection_level, array_size);
}