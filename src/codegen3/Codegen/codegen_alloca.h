#pragma once

#include "../CGExpr.h"
#include "../Codegen.h"
#include "ir/IR.h"

namespace cg
{
CGExpr codegen_decl_var(CG&, ir::Alloca*);
}
