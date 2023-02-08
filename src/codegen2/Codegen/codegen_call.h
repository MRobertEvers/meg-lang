#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "../LValue.cpp"
#include "LLVMFnInfo.h"
#include "LLVMFnSigInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> codegen_call(CG&, cg::LLVMFnInfo&, ir::IRCall*);
CGResult<CGExpr> codegen_call(CG&, cg::LLVMFnInfo&, ir::IRCall*, std::optional<LValue>);
} // namespace cg