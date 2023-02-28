#pragma once
#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "../LValue.cpp"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_enum(CG&, ir::IREnum*);
} // namespace cg