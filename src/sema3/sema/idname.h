
#pragma once

#include "../QualifiedName.h"
#include "ast2/Ast.h"

#include <string>

namespace sema
{
QualifiedName idname(ast::AstList<std::string*>& name_parts);

QualifiedName idname(ast::AstId const& id);
} // namespace sema