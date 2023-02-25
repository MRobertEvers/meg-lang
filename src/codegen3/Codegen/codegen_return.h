#pragma once
#include "../CGExpr.h"
#include "ir/IR.h"

namespace cg
{
class CG;

CGExpr codegen_return(CG&, ir::Return*);
} // namespace cg