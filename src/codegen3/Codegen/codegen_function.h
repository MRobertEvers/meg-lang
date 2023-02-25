#pragma once

#include "../CGExpr.h"
#include "LLVMFnInfo.h"
#include "ir/IR.h"

namespace cg
{
class CG;

CGExpr codegen_function(CG&, ir::Function*);
CGExpr codegen_function_proto(CG&, ir::FnDecl*);
} // namespace cg