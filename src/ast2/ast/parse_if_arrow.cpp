#include "parse_if_arrow.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"
#include "parse_struct.h"

#include <string>

using namespace ast;

AstNode*
ast::parse_if_arrow(AstGen& astgen)
{
	auto consume_tok = astgen.cursor.consume(TokenType::fat_arrow);
	if( !consume_tok.ok() )
		return ParseError("Expected arrow, '=>'.", consume_tok.as());
}