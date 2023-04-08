#pragma once

class Ty;

class QualifiedTy
{
public:
	Ty const* ty = nullptr;
	bool is_array = false;
	int array_size = 0;

	int indirection = 0;

	QualifiedTy();
	QualifiedTy(Ty const* ty);

	bool is_function() const;
	bool is_primitive() const;

	// TODO: compatible function not equals
	static bool equals(QualifiedTy lqty, QualifiedTy rqty);
};
