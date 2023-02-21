#pragma once

#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"

namespace cg
{
class CG;
CGResult<CGExpr> cg_fixdown(CG&, LLVMAddress const&, LLVMFixup const&);
} // namespace cg