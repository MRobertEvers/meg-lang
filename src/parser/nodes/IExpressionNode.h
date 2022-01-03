#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include <llvm/IR/Value.h>

namespace nodes
{
class IExpressionNode
{
public:
	virtual ~IExpressionNode() = default;

	virtual llvm::Value* codegen() = 0;
};
} // namespace nodes

#endif
