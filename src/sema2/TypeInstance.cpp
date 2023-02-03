#include "TypeInstance.h"

using namespace sema;
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