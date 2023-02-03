#include "Scope.h"
#include "ast2/AstTags.h"
#include "type/Type.h"

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
	Scope* scope;
};
} // namespace sema