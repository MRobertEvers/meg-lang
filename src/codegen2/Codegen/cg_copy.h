#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"

namespace cg
{
class CG;
CGResult<CGExpr> cg_copy(CG&, LLVMAddress& src);
} // namespace cg