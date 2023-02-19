#pragma once

#include "../Ast.h"
#include "../AstNode.h"
#include "../ParseResult.h"
#include "lexer/TokenCursor.h"

// TODO: This is unused?
namespace ast
{
class AstGen;

ParseResult<AstNode*> parse_is(AstGen&);

} // namespace ast