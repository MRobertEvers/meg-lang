#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
CGResult<CGExpr> cg_access(CG&, LLVMAddress src, sema::MemberTypeInstance const& member);
} // namespace cg