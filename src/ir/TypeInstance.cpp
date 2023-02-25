#include "TypeInstance.h"

#include "Type.h"

using namespace sema;

EnumNominal
TypeInstance::as_nominal() const
{
	assert(!is_array_ && indirection_level == 0);
	return type->as_nominal();
}

TypeInstance
TypeInstance::storage_type() const
{
	if( !is_array_ && indirection_level == 0 )
		return TypeInstance::OfType(type->get_dependent_type());
	else
		return *this;
}

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

bool
TypeInstance::is_enum_type() const
{
	return !is_array_ && indirection_level == 0 && type->is_enum_type();
}

bool
TypeInstance::is_union_type() const
{
	return !is_array_ && indirection_level == 0 && type->is_union_type();
}

TypeInstance
TypeInstance::Dereference() const
{
	assert(indirection_level > 0);
	return TypeInstance(this->type, this->indirection_level - 1);
}

TypeInstance
TypeInstance::PointerTo(int indirection) const
{
	return TypeInstance(this->type, this->indirection_level + indirection);
}

TypeInstance
TypeInstance::PointerElementType() const
{
	assert(!is_array_ && this->indirection_level > 0);
	return TypeInstance(this->type, this->indirection_level - 1);
}

TypeInstance
TypeInstance::ArrayElementType() const
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