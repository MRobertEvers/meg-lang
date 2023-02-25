#pragma once

#include "../CGExpr.h"
#include "../Codegen.h"
#include "ir/IR.h"

namespace cg
{
CGExpr codegen_inst(CG&, ir::Inst*);
}
