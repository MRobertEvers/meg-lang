#pragma once

#include "../Ast.h"
#include "../AstNode.h"
#include "../ParseResult.h"

namespace ast
{
class AstGen;
/**
 * @brief Parse a struct when the cursor is already pointing to the identifier.
 *
 * @return ParseResult<AstNode*>
 */
ParseResult<AstList<AstNode*>*> parse_struct_body(AstGen&);
ParseResult<AstNode*> parse_struct(AstGen&);

} // namespace ast