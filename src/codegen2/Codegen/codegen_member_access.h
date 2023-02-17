#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

CGResult<CGExpr> codegen_indirect_member_access(CG&, cg::LLVMFnInfo&, ir::IRIndirectMemberAccess*);
CGResult<CGExpr> codegen_member_access(CG&, cg::LLVMFnInfo&, ir::IRMemberAccess*);
} // namespace cg