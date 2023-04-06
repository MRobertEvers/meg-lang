#pragma once

#include "Ty.h"

class QualifiedTy
{
	Ty const* ty;

	bool is_array;
	int array_size;

	int indirection;

public:
	QualifiedTy(Ty const* ty);
};