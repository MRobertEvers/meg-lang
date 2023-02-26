#include "Sema.h"

using namespace sema;
using namespace ir;

static void
add_typename(ir::NameRef ns, ir::Type const* type)
{
	ns.add_name(Name(type->get_name(), TypeInstance::OfType(type), Name::Type));
}

Sema::Sema()
{
	add_typename(lookup_.root(), types_.i8_type());
	add_typename(lookup_.root(), types_.i16_type());
	add_typename(lookup_.root(), types_.i32_type());
	add_typename(lookup_.root(), types_.i64_type());
	add_typename(lookup_.root(), types_.u8_type());
	add_typename(lookup_.root(), types_.u16_type());
	add_typename(lookup_.root(), types_.u32_type());
	add_typename(lookup_.root(), types_.u64_type());
	add_typename(lookup_.root(), types_.bool_type());
	add_typename(lookup_.root(), types_.void_type());
}

NameRef
Sema::create_type(Type type_proto)
{
	return create_type(lookup_.current(), type_proto);
}

NameRef
Sema::create_type(NameRef nspace, Type type_proto)
{
	auto type = types_.define_type(type_proto);

	return nspace.add_name(
		Name(type->get_name(), nspace.id(), TypeInstance::OfType(type), Name::Type));
}