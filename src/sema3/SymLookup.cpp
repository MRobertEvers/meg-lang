#include "SymLookup.h"

SymLookupResult::SymLookupResult(std::vector<Sym*> syms)
	: syms(syms){};

bool
SymLookupResult::found() const
{
	return syms.size() > 0;
}

Sym*
SymLookupResult::first()
{
	return syms.size() > 0 ? syms.at(0) : nullptr;
}

Sym*
SymLookupResult::first_of(SymKind target)
{
	for( Sym* sym : syms )
	{
		if( sym->kind == target )
			return sym;
	}
	return nullptr;
}