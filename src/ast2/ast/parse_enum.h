#pragma once

#include "../Ast.h"
#include "../AstNode.h"
#include "../ParseResult.h"

namespace ast
{
class AstGen;

ParseResult<AstNode*> parse_enum(AstGen&);

} // namespace ast