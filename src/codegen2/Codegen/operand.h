#pragma once

#include "../CGExpr.h"
#include <llvm/IR/Value.h>

namespace cg
{
class CG;
llvm::Value* codegen_operand_expr(CG& codegen, CGExpr& result);
} // namespace cg