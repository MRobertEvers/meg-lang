#pragma once

#include "../IR.h"
#include "../SemaResult.h"

namespace sema
{
class Sema2;
SemaResult<ir::IRBlock*> lower_for(Sema2&, ir::IRFor* ir_for);
} // namespace sema