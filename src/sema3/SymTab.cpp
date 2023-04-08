#include "SymTab.h"

SymTab::SymTab()
{
	stack.push_back(&root);
};

SymLookupResult
SymTab::lookup(NameParts name)
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

SymLookupResult
SymTab::lookup(Ty const* ty)
{
	auto iter_find = ty_lookup.find(ty);
	if( iter_find != ty_lookup.end() )
		return SymLookupResult(iter_find->second);

	return SymLookupResult(nullptr);
}

void
SymTab::push_scope(SymScope* scope)
{
	stack.push_back(scope);
}

void
SymTab::pop_scope()
{
	stack.pop_back();
}

SymScope*
SymTab::current_scope()
{
	return stack.back();
}