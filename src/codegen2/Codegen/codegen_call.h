#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "../LValue.cpp"
#include "CGFunctionContext.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_call(CG&, cg::CGFunctionContext&, ir::IRCall*);
CGResult<CGExpr> codegen_call(CG&, cg::CGFunctionContext&, ir::IRCall*, std::optional<LValue>);
} // namespace cg