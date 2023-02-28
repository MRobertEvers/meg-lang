#include "sema_id.h"

#include "../sema_expected.h"
#include "idname.h"

using namespace sema;

SemaResult<sema_id_t>
sema::sema_id(Sema2& sema, ast::AstNode* ast)
{
	auto idr = expected(ast, ast::as_id);
	if( !idr.ok() )
		return idr;
	auto id = idr.unwrap();

	QualifiedName qname = idname(id);

	auto maybe_value = sema.lookup_local(qname);
	if( !maybe_value.is_found() )
		return SemaError("Use of undeclared identifier " + qname.to_string());

	sema::NameRef value = maybe_value.result();
	if( value.name().is_var() )
		return sema_id_t(sema.Id(ast, value, value.type(), false));

	if( value.name().is_type() )
	{
		auto type = value.type().type;
		if( type->is_struct_type() )
		{
			// Lookup constructor.
			// Note that we need to generate a constructor?
			return sema_id_t(sema.Id(ast, value, value.type(), true));
		}
		else if( type->get_dependent_type()->is_enum_type() )
		{
			return sema_id_t(sema.Initializer(ast, value, {}, TypeInstance::OfType(type)));
		}
		else
		{
			// TODO: Yikes
			return sema_id_t(sema.Id(ast, value, value.type(), true));
		}
	}

	// TODO: Leaks
	return SemaError("Unrecognized variable '" + qname.to_string() + "'");
}