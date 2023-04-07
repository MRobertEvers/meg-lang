#pragma once

class Ty;

class QualifiedTy
{
	Ty const* ty = nullptr;

	bool is_array = false;
	int array_size = 0;

	int indirection = 0;

public:
	QualifiedTy();
	QualifiedTy(Ty const* ty);
};