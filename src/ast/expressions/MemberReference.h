
#pragma once

#include "../IExpressionNode.h"
#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{
/**
 * @brief a.b
 *
 */
class MemberReference : public IExpressionNode
{
	Type const& type;

public:
	OwnPtr<IExpressionNode> base = nullptr;
	OwnPtr<ValueIdentifier> name = nullptr;

	MemberReference(OwnPtr<IExpressionNode> base, OwnPtr<ValueIdentifier> name, Type const& type)
		: base(std::move(base))
		, name(std::move(name))
		, type(type)
	{}

	Type const& get_type() const override { return type; }

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast