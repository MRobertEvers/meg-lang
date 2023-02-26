#pragma once
#include "../CGExpr.h"
#include "ir/IR.h"

namespace cg
{
class CG;

CGExpr codegen_var_ref(CG&, ir::VarRef*);
} // namespace cg