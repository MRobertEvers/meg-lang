#pragma once

#include "../ActionResult.h"
#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ir/Type.h"

namespace sema
{
SemaResult<ir::FnDecl*> sema_fn_proto(Sema& sema, ast::AstNode* ast);
SemaResult<ir::ActionResult> sema_fn(Sema& sema, ast::AstNode* ast);
SemaResult<ir::ActionResult> sema_extern_fn(Sema& sema, ast::AstNode* ast);
} // namespace sema