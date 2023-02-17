
#include "parse_enum.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"
#include "parse_struct.h"

#include <string>

using namespace ast;

using namespace ast;

static ParseResult<ast::AstNode*>
parse_enum_member(AstGen& astgen)
{}

ParseResult<ast::AstNode*>
ast::parse_enum(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

	auto members = astgen.ast.create_list();

	auto consume_tok = astgen.cursor.consume(TokenType::enum_keyword);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected 'enum'", consume_tok.as());
	}

	consume_tok = astgen.cursor.consume(TokenType::identifier);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected enum identifier.", consume_tok.as());
	}

	auto struct_name = to_type_identifier(astgen.ast, consume_tok, trail.mark());

	consume_tok = astgen.cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected block, '{'.", consume_tok.as());
	}

	auto tok = astgen.cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		consume_tok = astgen.cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected identifier.", consume_tok.as());
		}

		// consume_tok = astgen.cursor.consume_if_expected(TokenType::comma)
		// if (consume)
		// auto members = parse_struct_body(astgen);
	}

	consume_tok = astgen.cursor.consume(TokenType::close_curly);
	return ParseError("");
	// return astgen.ast.Struct(trail.mark(), struct_name, members.unwrap);
}
