#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_string_literal(CG&, ir::IRStringLiteral* lit);
} // namespace cg