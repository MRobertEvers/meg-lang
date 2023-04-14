#pragma once
#include "Sym.h"
#include "SymScope.h"
#include "ast3/NameParts.h"

#include <vector>

class SymLookupResult
{
	std::vector<Sym*> syms;

public:
	SymLookupResult(std::vector<Sym*> syms);

	bool found() const;
	Sym* first();
	Sym* first_of(SymKind);

	auto begin() { return syms.begin(); }
	auto end() { return syms.end(); }
};
