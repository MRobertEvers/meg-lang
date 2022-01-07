#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>
namespace ast
{
class Let : public IStatementNode
{
public:
	std::unique_ptr<TypeIdentifier> Type;
	std::unique_ptr<ValueIdentifier> Name;
	std::unique_ptr<IExpressionNode> RHS;

	Let(std::unique_ptr<ValueIdentifier> identifier,
		std::unique_ptr<TypeIdentifier> type,
		std::unique_ptr<IExpressionNode> rhs)
		: Name(std::move(identifier))
		, Type(std::move(type))
		, RHS(std::move(rhs)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast