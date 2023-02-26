#pragma once
#include "../CGExpr.h"
#include "../Codegen.h"
#include "LValue.h"
#include "ir/IR.h"

namespace cg
{
CGExpr codegen_call(CG&, ir::FnCall*);
CGExpr codegen_call(CG&, ir::FnCall*, std::optional<LValue> lvalue);
} // namespace cg
