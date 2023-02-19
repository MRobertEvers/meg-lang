#pragma once

#include "../Ast.h"
#include "../AstNode.h"
#include "../ParseResult.h"
#include "lexer/TokenCursor.h"

namespace ast
{
class AstGen;

AstNode* to_value_identifier(Ast& ast, ConsumeResult const& tok_res, Span span);

} // namespace ast