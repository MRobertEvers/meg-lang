#pragma once

class Ty;

class QualifiedTy
{
	bool is_array_ = false;

public:
	Ty const* ty = nullptr;
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
	bool is_struct() const;
	bool is_union() const;

	/**
	 * Pointers and arrays
	 *
	 * @return true
	 * @return false
	 */
	bool is_subscriptable() const;

	/**
	 * Aggregate types are treated differently
	 * in function passing.
	 *
	 * @return true
	 * @return false
	 */
	bool is_aggregate_type() const;

	QualifiedTy deref() const;
	QualifiedTy pointer_to() const;

	// TODO: compatible function not equals
	static bool equals(QualifiedTy lqty, QualifiedTy rqty);
};
