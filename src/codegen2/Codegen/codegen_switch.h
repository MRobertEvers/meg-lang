#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_switch(CG&, cg::LLVMFnInfo&, ir::IRSwitch*);

CGResult<CGExpr> codegen_case(CG&, cg::LLVMFnInfo&, ir::IRCase*);
} // namespace cg