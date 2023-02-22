#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "common/Vec.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
void cg_discriminations(CG&, CGExpr&, Vec<ir::IRParam*>&);
} // namespace cg