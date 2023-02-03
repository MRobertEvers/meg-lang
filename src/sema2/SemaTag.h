#include "Scope.h"
#include "Type.h"
#include "ast2/AstTags.h"

namespace sema
{
enum SemaTagType
{
	Default,
	Sema
};

class SemaTag : public ast::IAstTag
{
public:
	SemaTagType tag_type = SemaTagType::Default;
	TypeInstance type;
	Scope* scope;
};
} // namespace sema