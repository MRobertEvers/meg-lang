#include "parse_if_arrow.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"
#include "parse_struct.h"

#include <string>

using namespace ast;

ParseResult<AstNode*>
ast::parse_if_arrow(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

	auto consume_tok = astgen.cursor.consume(TokenType::fat_arrow);
	if( !consume_tok.ok() )
		return ParseError("Expected arrow, '=>'.", consume_tok.as());

	consume_tok = astgen.cursor.consume(TokenType::open_paren);
	if( !consume_tok.ok() )
		return ParseError("Expected '('", consume_tok.as());

	auto param_list = astgen.parse_function_parameter_list();
	if( !param_list.ok() )
		return param_list;

	consume_tok = astgen.cursor.consume(TokenType::close_paren);
	if( !consume_tok.ok() )
		return ParseError("Expected ')'", consume_tok.as());

	auto block = astgen.parse_block();
	if( !block.ok() )
		return block;

	return astgen.ast.IfArrow(trail.mark(), param_list.unwrap(), block.unwrap());
}