#include "SymScope.h"

Sym*
SymScope::find(std::string const& name)
{
	auto iter_find = syms.find(name);
	if( iter_find != syms.end() )
		return iter_find->second;

	return nullptr;
}

void
SymScope::insert(std::string const& name, Sym* sym)
{
	syms.emplace(name, sym);
}