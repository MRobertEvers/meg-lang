#pragma once

#include "../Ast.h"
#include "../AstNode.h"
#include "../ParseResult.h"
#include "lexer/TokenCursor.h"

namespace ast
{
class AstGen;

AstNode* parse_if_arrow(AstGen&);

} // namespace ast