#ifndef IEXPRESSION_NODE_H_
#define IEXPRESSION_NODE_H_

#include "IStatementNode.h"

namespace ast
{
class IExpressionNode : public IStatementNode
{
public:
	IExpressionNode(Span span)
		: IStatementNode(span){};
	virtual ~IExpressionNode() = default;
};
} // namespace ast

#endif
