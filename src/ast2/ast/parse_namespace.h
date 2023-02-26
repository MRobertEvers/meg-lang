#pragma once

#include "../AstNode.h"
#include "../ParseResult.h"

namespace ast
{
class AstGen;

ParseResult<AstNode*> parse_namespace(AstGen&);
} // namespace ast