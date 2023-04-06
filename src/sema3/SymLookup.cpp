#include "SymLookup.h"

SymLookupResult::SymLookupResult(Sym* sym)
	: sym(sym){};

SymLookup::SymLookup()
{
	stack.push_back(&root);
};

SymLookupResult
SymLookup::lookup(NameParts name)
{
	std::vector<SymScope*> search_stack = stack;
	while( search_stack.size() != 0 )
	{
		SymScope* scope = search_stack.back();
		search_stack.pop_back();

		for( auto& part : name.parts )
		{
			// TODO: Lookup other parts based on the symbol returned.
			Sym* sym = scope->find(part);
			if( sym )
				return SymLookupResult(sym);
		}
	}

	return SymLookupResult(nullptr);
}