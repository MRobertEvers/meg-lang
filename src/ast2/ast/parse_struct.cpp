#include "parse_struct.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"

#include <string>

using namespace ast;

ParseResult<AstList<AstNode*>*>
ast::parse_struct_body(AstGen& astgen)
{
	auto members = astgen.ast.create_list();

	auto consume_tok = astgen.cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected block, '{'.", consume_tok.as());
	}

	auto tok = astgen.cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		auto member_trail = astgen.get_parse_trail();

		consume_tok = astgen.cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected member declaration.", consume_tok.as());
		}

		auto name = to_value_identifier(astgen.ast, consume_tok, member_trail.mark());

		consume_tok = astgen.cursor.consume(TokenType::colon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ':'.", consume_tok.as());
		}

		auto decl = astgen.parse_type_decl(false);
		if( !decl.ok() )
		{
			return ParseError(*decl.unwrap_error());
		}

		consume_tok = astgen.cursor.consume(TokenType::semicolon, TokenType::close_curly);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ';'.", consume_tok.as());
		}

		members->append(astgen.ast.ValueDecl(member_trail.mark(), name, decl.unwrap()));

		if( consume_tok.as().type == TokenType::close_curly )
			break;

		tok = astgen.cursor.peek();
	}
	if( consume_tok.as().type != TokenType::close_curly )
		consume_tok = astgen.cursor.consume(TokenType::close_curly);

	return members;
}

ParseResult<ast::AstNode*>
ast::parse_struct(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

	auto consume_tok = astgen.cursor.consume(TokenType::struct_keyword);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected 'struct'", consume_tok.as());
	}

	consume_tok = astgen.cursor.consume(TokenType::identifier);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected struct identifier.", consume_tok.as());
	}

	auto struct_name = to_type_identifier(astgen.ast, consume_tok, trail.mark());

	auto members = parse_struct_body(astgen);
	if( !members.ok() )
	{
		return ParseError(*members.unwrap_error());
	}

	return astgen.ast.Struct(trail.mark(), struct_name, members.unwrap());
}
