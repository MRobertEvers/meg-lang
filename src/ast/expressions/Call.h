
#pragma once

#include "../IExpressionNode.h"
#include "Identifier.h"
#include "common/OwnPtr.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{
/**
 * @brief a()
 *
 */
class Call : public IExpressionNode
{
public:
	Vec<OwnPtr<IExpressionNode>> args;
	OwnPtr<TypeIdentifier> name = nullptr;

	Call(OwnPtr<TypeIdentifier> name, Vec<OwnPtr<IExpressionNode>>& args)
		: name(std::move(name))
		, args(std::move(args))
	{}

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast