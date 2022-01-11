#pragma once

#include "../IExpressionNode.h"
#include "../Type.h"
#include "common/OwnPtr.h"

namespace ast
{

class TypeDeclarator : public IExpressionNode
{
	OwnPtr<TypeDeclarator> base = nullptr;

	// This must be declared before `type` because the `type` field
	// might reference this field.
	OwnPtr<Type> pointer_to_type = nullptr;
	/**
	 * @brief
	 * The type is owned by the TypeIdentifier. PointerTo's are owned by the declarator.
	 *
	 * That means each instance of my_type* is a unique Type object.
	 */
	Type const& type;

public:
	// Constructor of pointer to type
	explicit TypeDeclarator(Type const& type)
		: type(type)
	{}

	bool is_pointer_type() const { return type.is_pointer_type(); }

	String get_fqn() const
	{
		if( is_pointer_type() )
		{
			return "*";
		}
		else
		{
			return type.name;
		}
	}

	Type const& get_type() const override { return type; }

	TypeDeclarator const* get_base() const { return base.get(); }

	bool is_empty() const { return type.name == infer_type.name; }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeDeclarator()
		: type(infer_type)
	{}

	TypeDeclarator(OwnPtr<TypeDeclarator> base)
		: base(std::move(base))
		, pointer_to_type(Type::PointerTo(this->base->type))
		, type(*pointer_to_type.get())
	{}

public:
	static OwnPtr<TypeDeclarator> Empty() { return new TypeDeclarator(); }

	static OwnPtr<TypeDeclarator> PointerToTy(OwnPtr<TypeDeclarator> base)
	{
		return new TypeDeclarator(std::move(base));
	}
};
} // namespace ast