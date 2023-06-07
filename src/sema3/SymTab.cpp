#include "SymTab.h"

SymTab::SymTab()
{
	stack.push_back(&root);
};

SymLookupResult
SymTab::lookup(NameParts name)
{
	std::vector<SymScope*> search_stack = stack;

	std::vector<Sym*> syms;
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
			{
				syms.push_back(sym);
				break;
			}

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

	return SymLookupResult(syms);
}

SymLookupResult
SymTab::lookup(Ty const* ty)
{
	auto iter_find = ty_lookup.find(ty);
	if( iter_find != ty_lookup.end() )
		return SymLookupResult({iter_find->second});

	return SymLookupResult({});
}

static bool
param_match(std::vector<QualifiedTy> left, std::vector<QualifiedTy> right)
{
	if( left.size() != right.size() )
		return false;

	for( int i = 0; i < right.size(); i++ )
	{
		QualifiedTy& l_qty = left.at(i);
		QualifiedTy& r_qty = right.at(i);
		if( !QualifiedTy::equals(l_qty, r_qty) )
			return false;
	}

	return true;
}

static void
find_matches(Sym* sym, std::vector<QualifiedTy> params, std::vector<Sym*>& out_matches)
{
	assert(sym->kind == SymKind::Template);

	SymTemplate& templ = sym_cast<SymTemplate>(sym);

	for( auto& instance : templ.instances )
	{
		if( param_match(params, instance.types) )
			out_matches.push_back(instance.sym);
	}
}

SymLookupResult
SymTab::lookup_template_instance(NameParts name, std::vector<QualifiedTy> params)
{
	std::vector<Sym*> filtered;
	SymLookupResult lu = lookup(name);
	if( !lu.found() )
		return lu;

	for( Sym* sym : lu )
	{
		if( sym->kind == SymKind::Template )
			find_matches(sym, params, filtered);
	}

	return SymLookupResult(filtered);
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

std::vector<SymScope*>
SymTab::save_state()
{
	std::vector<SymScope*> out = stack;

	stack = {&root};

	return out;
}

void
SymTab::restore_state(std::vector<SymScope*> in)
{
	stack = in;
}

Sym*
SymTab::clone_symbol_to(SymScope* scope, std::string const& name, Sym* base)
{
	Sym& cloned = syms.emplace_back();
	memcpy(&cloned, base, sizeof(Sym));

	scope->insert(name, &cloned);

	return &cloned;
}

SymScope*
SymTab::current_scope()
{
	return stack.back();
}