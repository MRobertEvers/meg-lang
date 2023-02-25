#pragma once

#include "../ActionResult.h"
#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"

namespace sema
{
SemaResult<ir::TypeInstance> sema_type_decl(Sema&, ast::AstNode*);
}