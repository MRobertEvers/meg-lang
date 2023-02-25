#include "SemaLookup.h"

using namespace sema;
using namespace ir;

SemaLookup::SemaLookup()
	: root_namespace(NameRef(&names, 0))
{
	names.push_back(Name(""));
}

NameLookupResult
SemaLookup::lookup(QualifiedName const& name)
{
	//
	if( name.length() == 0 )
		return NameLookupResult();

	NameRef it = root_namespace;

	for( int i = 0; i < name.length(); i++ )
	{
		auto maybe_name = it.lookup(name.part(i));
		if( !maybe_name.has_value() )
			return NameLookupResult();

		it = maybe_name.value();
	}

	return NameLookupResult(it);
}

void
SemaLookup::add_name(NameRef nspace, Name name)
{
	nspace.add_name(name);
}