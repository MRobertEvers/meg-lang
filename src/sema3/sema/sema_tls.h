#pragma once

#include "../ActionResult.h"
#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"

namespace sema
{
SemaResult<ir::ActionResult> sema_tls(Sema& sema, ast::AstNode* ast);
} // namespace sema