#pragma once

#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ir/ActionResult.h"

namespace sema
{
SemaResult<ir::ActionResult> sema_fn_proto(Sema& sema, ast::AstNode* ast);
SemaResult<ir::ActionResult> sema_fn(Sema& sema, ast::AstNode* ast);
SemaResult<ir::ActionResult> sema_extern_fn(Sema& sema, ast::AstNode* ast);
} // namespace sema