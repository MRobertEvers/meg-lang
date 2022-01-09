#pragma once
#include "IAstVisitor.h"
#include "ast_node_type.h"

namespace ast
{
class IAstNode
{
public:
	virtual ~IAstNode() = default;

	virtual void visit(IAstVisitor* visitor) const = 0;
};
} // namespace ast