#pragma once

#include "../IExpressionNode.h"
#include "../Type.h"
#include "common/OwnPtr.h"

namespace ast
{
class TypeDeclarator : public IExpressionNode
{
	OwnPtr<TypeDeclarator> base = nullptr;

	String name;

public:
	TypeDeclarator(String& name)
		: name(name)
	{}
	TypeDeclarator(String&& name)
		: name(name)
	{}

	bool is_pointer_type() const { return !base.is_null(); }

	String get_name() const
	{
		if( is_pointer_type() )
		{
			return "*";
		}
		else
		{
			return base->get_name();
		}
	}

	bool is_empty() const { return !name.empty(); }

	TypeDeclarator const* get_base() const { return base.get(); }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeDeclarator() {}

	TypeDeclarator(OwnPtr<TypeDeclarator> base)
		: base(std::move(base))
	{}

public:
	static OwnPtr<TypeDeclarator> Empty() { return new TypeDeclarator(); }

	static OwnPtr<TypeDeclarator> PointerToTy(OwnPtr<TypeDeclarator> base)
	{
		return new TypeDeclarator(std::move(base));
	}
};
} // namespace ast