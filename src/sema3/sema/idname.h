
#pragma once

#include "ast2/Ast.h"
#include "ir/QualifiedName.h"

#include <string>

namespace sema
{
ir::QualifiedName idname(ast::AstList<std::string*>& name_parts);

ir::QualifiedName idname(ast::AstId const& id);
} // namespace sema