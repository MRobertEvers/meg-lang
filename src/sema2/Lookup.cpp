#include "Lookup.h"

using namespace sema;

Lookup::Lookup()
	: root_namespace(NameRef(&names, 0))

{
	names.push_back(Name(""));
	namespace_stack.push_back(root_namespace);
}

NameLookupResult
Lookup::lookup_fqn(QualifiedName const& name)
{
	return lookup(name, root_namespace);
}

NameLookupResult
Lookup::lookup(QualifiedName const& name)
{
	return lookup(name, current());
}

NameLookupResult
Lookup::lookup(QualifiedName const& name, sema::NameRef nspace)
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

sema::NameRef
Lookup::get(sema::NameId name_id)
{
	assert(name_id.index() < this->names.size());
	return NameRef(&this->names, name_id);
}

sema::NameRef
Lookup::add_name(Name name)
{
	return add_name(current(), name);
}

sema::NameRef
Lookup::add_name(sema::NameRef nspace, Name name)
{
	return nspace.add_name(name);
}

sema::NameRef
Lookup::push_scope(sema::NameRef nspace)
{
	namespace_stack.push_back(nspace);
	return nspace;
}

void
Lookup::pop_scope()
{
	namespace_stack.pop_back();
}