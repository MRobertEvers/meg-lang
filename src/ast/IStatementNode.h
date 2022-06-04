#ifndef ISTATEMENT_NODE_H_
#define ISTATEMENT_NODE_H_

#include "IAstNode.h"
#include "Span.h"

namespace ast
{
class IStatementNode : public IAstNode
{
public:
	IStatementNode(Span span)
		: IAstNode(span){};
	virtual ~IStatementNode() = default;
};
} // namespace ast

#endif