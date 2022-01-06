#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>
namespace ast
{
class Let : public IStatementNode
{
public:
	Identifier identifier;
	Identifier type;
	std::unique_ptr<IExpressionNode> rhs;

	Let(Identifier identifier, Identifier type, std::unique_ptr<IExpressionNode> rhs)
		: identifier(identifier)
		, type(type)
		, rhs(std::move(rhs)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast