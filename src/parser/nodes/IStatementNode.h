#ifndef ISTATEMENT_NODE_H_
#define ISTATEMENT_NODE_H_

#include <llvm/IR/Value.h>

namespace nodes
{
class IStatementNode
{
public:
	virtual ~IStatementNode() = default;

	virtual void codegen() = 0;
};
} // namespace nodes

#endif