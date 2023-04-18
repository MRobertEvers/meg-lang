#pragma once

class Ty;

class QualifiedTy
{
public:
	Ty const* ty = nullptr;
	bool is_array = false;
	int array_size = 0;

	int indirection = 0;

	// note: impl T* is
	// actually (impl T)*
	// or a pointer to T*.
	enum class ImplKind
	{
		Impl,
		None,
	} impl = ImplKind::Impl;

	QualifiedTy();
	QualifiedTy(Ty const* ty);
	QualifiedTy(Ty const* ty, int indirection);

	// Doesn't really make sense to have a pointer to an impl.
	// Then it's just a pointer...
	QualifiedTy(Ty const* ty, ImplKind kind);

	// bool is_array() const;
	bool is_int() const;
	bool is_pointer() const;
	bool is_function() const;
	bool is_enum() const;

	QualifiedTy deref() const;

	// TODO: compatible function not equals
	static bool equals(QualifiedTy lqty, QualifiedTy rqty);
};
