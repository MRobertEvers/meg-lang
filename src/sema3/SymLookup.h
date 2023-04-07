#pragma once
#include "SymScope.h"
#include "ast3/NameParts.h"

#include <vector>

struct SymLookupResult
{
	Sym* sym;
	SymLookupResult(Sym* sym);
};
