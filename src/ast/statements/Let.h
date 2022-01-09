#include "../IExpressionNode.h"
#include "../IStatementNode.h"

#include <memory>
namespace ast
{
class Let : public IStatementNode
{
public:
	OwnPtr<TypeIdentifier> Type;
	OwnPtr<ValueIdentifier> Name;
	OwnPtr<IExpressionNode> RHS;

	Let(OwnPtr<ValueIdentifier> identifier,
		OwnPtr<TypeIdentifier> type,
		OwnPtr<IExpressionNode> rhs)
		: Name(std::move(identifier))
		, Type(std::move(type))
		, RHS(std::move(rhs)){};

	virtual void visit(IAstVisitor* visitor) const override { return visitor->visit(this); };
};
} // namespace ast