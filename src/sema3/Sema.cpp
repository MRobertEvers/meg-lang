#include "Sema.h"

using namespace sema;
using namespace ir;

Sema::Sema()
{
	lookup_.root().add_name(
		Name(types_.i8_type()->get_name(), TypeInstance::OfType(types_.i8_type()), Name::Type));
	lookup_.root().add_name(
		Name(types_.i32_type()->get_name(), TypeInstance::OfType(types_.i32_type()), Name::Type));
	lookup_.root().add_name(
		Name(types_.void_type()->get_name(), TypeInstance::OfType(types_.void_type()), Name::Type));
}

NameRef
Sema::create_type(Type type_proto)
{
	return create_type(lookup_.root(), type_proto);
}

NameRef
Sema::create_type(NameRef nspace, Type type_proto)
{
	auto type = types_.define_type(type_proto);

	return nspace.add_name(Name(type->get_name(), TypeInstance::OfType(type), Name::Type));
}