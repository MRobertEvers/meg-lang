#pragma once

#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ir/ActionResult.h"

namespace sema
{
SemaResult<ir::ActionResult> sema_tls(Sema& sema, ast::AstNode* ast);
} // namespace sema