#pragma once
#include "../IR.h"
#include "../Sema2.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ast2/AstCasts.h"

namespace sema
{
SemaResult<ir::IRNamespace*> sema_namespace(Sema2& sema, ast::AstNode* ast);
}