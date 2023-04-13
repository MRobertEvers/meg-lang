#include "SymTab.h"

SymTab::SymTab()
{
	stack.push_back(&root);
};

SymLookupResult
SymTab::lookup(NameParts name)
{
	std::vector<SymScope*> search_stack = stack;
	int last_part = name.parts.size();
	while( search_stack.size() != 0 )
	{
		SymScope* scope = search_stack.back();
		search_stack.pop_back();

		for( int i = 0; i < last_part; i++ )
		{
			std::string& part = name.parts.at(i);

			Sym* sym = scope->find(part);
			if( !sym )
				break;

			if( i == last_part - 1 )
				return SymLookupResult(sym_unalias(sym));

			switch( sym->kind )
			{
			case SymKind::Type:
				scope = &sym->data.sym_type.scope;
				break;
			case SymKind::Namespace:
				scope = &sym->data.sym_namespace.scope;
				break;
			default:
				break;
			}
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
SymTab::push_scope()
{
	free_scopes.emplace_back();
	stack.push_back(&free_scopes.back());
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