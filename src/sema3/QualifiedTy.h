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
	QualifiedTy(Ty const* ty, int indirection);

	// bool is_array() const;
	bool is_pointer() const;
	bool is_function() const;
	bool is_primitive() const;
	bool is_enum() const;

	QualifiedTy deref() const;

	// TODO: compatible function not equals
	static bool equals(QualifiedTy lqty, QualifiedTy rqty);
};
