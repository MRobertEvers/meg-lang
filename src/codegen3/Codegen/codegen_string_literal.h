#include "../CGExpr.h"
#include "ir/IR.h"

namespace cg
{
class CG;
CGExpr codegen_string_literal(CG&, ir::StringLiteral*);
} // namespace cg