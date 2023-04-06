#pragma once
#include "SymScope.h"
#include "ast3/NameParts.h"

#include <vector>

struct SymLookupResult
{
	Sym* sym;
	SymLookupResult(Sym* sym);
};

class SymLookup
{
	SymScope root;

	std::vector<SymScope*> stack;

	// TODO: Reverse type lookup
public:
	SymLookup();

	SymLookupResult lookup(NameParts name);
};