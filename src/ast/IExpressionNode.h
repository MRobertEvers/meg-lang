#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include "../parser/CodegenContext.h"
#include <llvm/IR/Value.h>

namespace ast
{
class IExpressionNode
{
public:
	virtual ~IExpressionNode() = default;

	virtual llvm::Value* codegen(CodegenContext& codegen) = 0;
};
} // namespace ast

#endif
