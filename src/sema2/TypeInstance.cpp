#include "TypeInstance.h"

// TODO: Fix circular deps.
#include "type/Type.h"

using namespace sema;

bool
TypeInstance::is_function_type() const
{
	return indirection_level == 0 && type->is_function_type();
}

bool
TypeInstance::is_struct_type() const
{
	return indirection_level == 0 && type->is_struct_type();
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
