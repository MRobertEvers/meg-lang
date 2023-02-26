#pragma once
#include "../CGExpr.h"
#include "ir/IR.h"

namespace cg
{
class CG;

CGExpr codegen_store(CG&, ir::Store*);
} // namespace cg