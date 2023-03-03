#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"

namespace cg
{
class CG;
CGResult<CGExpr> cg_copy_alloca(CG&, LLVMAddress& src);
CGExpr cg_copy(CG&, LLVMAddress& src, LLVMAddress& dest);
} // namespace cg