#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include "../CodegenContext.h"
#include <llvm/IR/Value.h>

namespace nodes
{
class IExpressionNode
{
public:
	virtual ~IExpressionNode() = default;

	virtual llvm::Value* codegen(CodegenContext& codegen) = 0;
};
} // namespace nodes

#endif
