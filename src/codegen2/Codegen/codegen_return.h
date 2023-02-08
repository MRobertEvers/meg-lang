#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "CGFunctionContext.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_return(CG&, cg::CGFunctionContext&, ir::IRReturn*);
} // namespace cg