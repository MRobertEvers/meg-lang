#ifndef ISTATEMENT_NODE_H_
#define ISTATEMENT_NODE_H_

#include "../CodegenContext.h"
#include <llvm/IR/Value.h>

namespace ast
{
class IStatementNode
{
public:
	virtual ~IStatementNode() = default;

	virtual void codegen(CodegenContext& codegen) = 0;
};
} // namespace ast

#endif