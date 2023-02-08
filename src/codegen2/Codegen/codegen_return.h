#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnSigInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_return(CG&, cg::LLVMFnSigInfo&, ir::IRReturn*);
} // namespace cg