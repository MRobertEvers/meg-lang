#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include "IStatementNode.h"
#include "Type.h"
#include <llvm/IR/Value.h>

namespace ast
{
class IExpressionNode : public IStatementNode
{
public:
	virtual ~IExpressionNode() = default;
};
} // namespace ast

#endif
