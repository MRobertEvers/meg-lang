#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

#include <optional>

namespace cg
{
class CG;

CGResult<CGExpr>
codegen_initializer(CG&, cg::LLVMFnInfo&, ir::IRInitializer*, std::optional<LValue>);
} // namespace cg