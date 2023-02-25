#include "Sema.h"

using namespace sema;

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