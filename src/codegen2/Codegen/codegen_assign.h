#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_assign(CG&, cg::LLVMFnInfo&, ir::IRAssign*);
} // namespace cg