#include "SemaLookup.h"

using namespace sema;
using namespace ir;

SemaLookup::SemaLookup()
	: root_namespace(NameRef(&names, 0))

{
	names.push_back(Name(""));
	namespace_stack.push_back(root_namespace);
}

NameLookupResult
SemaLookup::lookup(QualifiedName const& name)
{
	return lookup(name, root_namespace);
}

NameLookupResult
SemaLookup::lookup_decl(QualifiedName const& name)
{
	return lookup(name, current());
}

NameLookupResult
SemaLookup::lookup(QualifiedName const& name, ir::NameRef nspace)
{
	if( name.length() == 0 )
		return NameLookupResult();

	NameRef it = nspace;

	for( int i = 0; i < name.length(); i++ )
	{
		auto maybe_name = it.lookup(name.part(i));
		if( !maybe_name.has_value() )
			return NameLookupResult();

		it = maybe_name.value();
	}

	return NameLookupResult(it);
}

ir::NameRef
SemaLookup::add_name(Name name)
{
	return add_name(current(), name);
}

ir::NameRef
SemaLookup::add_name(ir::NameRef nspace, Name name)
{
	return nspace.add_name(name);
}

ir::NameRef
SemaLookup::push_scope(ir::NameRef nspace)
{
	namespace_stack.push_back(nspace);
	return nspace;
}

void
SemaLookup::pop_scope()
{
	namespace_stack.pop_back();
}
