#ifndef ISTATEMENT_NODE_H_
#define ISTATEMENT_NODE_H_

#include "../parser/CodegenContext.h"
#include "IAstNode.h"
#include <llvm/IR/Value.h>

namespace ast
{
class IStatementNode : public IAstNode
{
public:
	virtual ~IStatementNode() = default;
};
} // namespace ast

#endif