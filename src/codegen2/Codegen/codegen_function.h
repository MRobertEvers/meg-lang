#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_function(CG&, ir::IRFunction*);
CGResult<LLVMFnInfo> codegen_function_entry(CG&, cg::LLVMFnSigInfo& fn_info);
CGResult<LLVMFnSigInfo> codegen_function_proto(CG&, ir::IRProto*);
CGResult<CGExpr> codegen_function_body(CG&, cg::LLVMFnInfo&, ir::IRBlock*);
} // namespace cg