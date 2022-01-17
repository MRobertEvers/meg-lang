#pragma once

#include "../IExpressionNode.h"
#include "common/OwnPtr.h"

namespace ast
{
class TypeDeclarator : public IExpressionNode
{
	OwnPtr<TypeDeclarator> base = nullptr;

	String name;

public:
	TypeDeclarator(Span span, String& name)
		: IExpressionNode(span)
		, name(name)
	{}
	TypeDeclarator(Span span, String&& name)
		: IExpressionNode(span)
		, name(name)
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
			return name;
		}
	}

	String get_type_name() const
	{
		if( is_pointer_type() )
		{
			return base->get_type_name();
		}
		else
		{
			return name;
		}
	}

	bool is_empty() const { return name.empty(); }

	TypeDeclarator const* get_base() const { return base.get(); }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };

private:
	TypeDeclarator()
		: IExpressionNode(Span{})
	{}

	TypeDeclarator(Span span, OwnPtr<TypeDeclarator> base)
		: IExpressionNode(span)
		, base(std::move(base))
	{}

public:
	static OwnPtr<TypeDeclarator> Empty() { return new TypeDeclarator(); }

	static OwnPtr<TypeDeclarator> PointerToTy(Span span, OwnPtr<TypeDeclarator> base)
	{
		return new TypeDeclarator(span, std::move(base));
	}
};
} // namespace ast