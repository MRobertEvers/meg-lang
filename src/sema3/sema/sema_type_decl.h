#pragma once

#include "../Sema.h"
#include "../SemaResult.h"
#include "../ir/ActionResult.h"
#include "ast2/Ast.h"

namespace sema
{
SemaResult<TypeInstance> sema_type_decl(Sema&, ast::AstNode*);
}