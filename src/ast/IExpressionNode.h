#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include "../parser/CodegenContext.h"
#include "IAstNode.h"
#include <llvm/IR/Value.h>

namespace ast
{
class IExpressionNode : public IAstNode
{
public:
	virtual ~IExpressionNode() = default;
};
} // namespace ast

#endif
