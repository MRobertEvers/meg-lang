#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnSigInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_function(CG&, ir::IRFunction*);
CGResult<LLVMFnSigInfo> codegen_function_proto(CG&, ir::IRProto*);
CGResult<CGExpr> codegen_function_body(CG&, cg::LLVMFnSigInfo&, ir::IRBlock*);
} // namespace cg