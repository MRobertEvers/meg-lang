#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"

namespace cg
{
class CG;
LLVMAddress cg_enum_nominal(CG&, LLVMAddress const&);
} // namespace cg