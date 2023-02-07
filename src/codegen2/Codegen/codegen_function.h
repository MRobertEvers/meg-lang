#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "CGFunctionContext.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_function(CG&, ir::IRFunction*);
CGResult<CGFunctionContext> codegen_function_proto(CG&, ir::IRProto*);
CGResult<CGExpr> codegen_function_body(CG&, cg::CGFunctionContext&, ir::IRBlock*);
} // namespace cg