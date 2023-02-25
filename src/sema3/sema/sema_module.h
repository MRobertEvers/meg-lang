#pragma once

#include "../Sema.h"
#include "../SemaResult.h"
#include "ast2/Ast.h"
#include "ir/IR.h"

namespace sema
{
SemaResult<ir::BasicBlock*> sema_module(Sema& sema, ast::AstNode* ast);
}