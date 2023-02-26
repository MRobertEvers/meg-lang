#pragma once

#include "../ActionResult.h"
#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ir/IR.h"

namespace sema
{
SemaResult<ir::ActionResult> sema_id(Sema& sema, ast::AstNode* ast);
}