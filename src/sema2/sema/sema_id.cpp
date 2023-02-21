#include "sema_id.h"

#include "../sema_expected.h"

using namespace sema;

static String*
to_single_name(Sema2& sema, ast::AstList<String*>* list)
{
	auto name = sema.create_name("", 0);

	bool first = true;
	for( auto part : list )
	{
		if( !first )
			*name += "#";
		*name += *part;

		first = false;
	}

	return name;
}

SemaResult<sema_id_t>
sema::sema_id(Sema2& sema, ast::AstNode* ast)
{
	auto idr = expected(ast, ast::as_id);
	if( !idr.ok() )
		return idr;
	auto id = idr.unwrap();

	// TODO: Leaks
	auto maybe_value = sema.lookup_name(*to_single_name(sema, id.name_parts));
	if( maybe_value.has_value() )
		// TODO: Allocate new name instead of reference
		return sema_id_t(sema.Id(ast, &id.name_parts->list, maybe_value.value(), false));

	// Struct name?
	// TODO: Leaks
	auto maybe_struct = sema.lookup_type(*to_single_name(sema, id.name_parts));
	if( maybe_struct )
	{
		if( maybe_struct->is_struct_type() )
		{
			// Lookup constructor.
			// Note that we need to generate a constructor?
			auto str_name = maybe_struct->get_name();
			auto name = sema.create_name(str_name.c_str(), str_name.size());
			auto parts = new Vec<String*>();
			parts->push_back(name);
			return sema_id_t(sema.Id(ast, parts, TypeInstance::OfType(maybe_struct), true));
		}
		else if( maybe_struct->get_dependent_type()->is_enum_type() )
		{
			return sema_id_t(sema.Initializer(
				ast, to_single_name(sema, id.name_parts), {}, TypeInstance::OfType(maybe_struct)));
		}
		else
		{
			// TODO: Yikes
			return sema_id_t(
				sema.Id(ast, &id.name_parts->list, TypeInstance::OfType(maybe_struct), true));
		}
	}

	// TODO: Leaks
	return SemaError("Unrecognized variable '" + *to_single_name(sema, id.name_parts) + "'");
}