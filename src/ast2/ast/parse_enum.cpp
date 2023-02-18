
#include "parse_enum.h"

#include "../Ast.h"
#include "../AstGen.h"
#include "../bin_op.h"
#include "parse_common.h"
#include "parse_struct.h"

#include <string>

using namespace ast;

static ParseResult<ast::AstNode*>
parse_enum_member(AstGen& astgen)
{}

ParseResult<ast::AstNode*>
ast::parse_enum(AstGen& astgen)
{
	auto trail = astgen.get_parse_trail();

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

	auto members = astgen.ast.create_list();
	auto tok = astgen.cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		auto member_trail = astgen.get_parse_trail();
		consume_tok = astgen.cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected identifier.", consume_tok.as());
		}
		auto member_name_node = to_type_identifier(astgen.ast, consume_tok, trail.mark());
		auto name_tok = consume_tok.unwrap();

		auto comma_tok = astgen.cursor.consume_if_expected(TokenType::comma);
		if( comma_tok.ok() )
		{
			auto member_name = astgen.ast.create_string(name_tok.start, name_tok.size);
			members->append(astgen.ast.EnumMemberEmpty(member_trail.mark(), member_name));
		}
		else
		{
			auto struct_members = parse_struct_body(astgen);
			if( !struct_members.ok() )
			{
				return ParseError(*struct_members.unwrap_error());
			}
			auto member_struct_name =
				to_type_identifier(astgen.ast, consume_tok, member_trail.mark());
			auto as_struct =
				astgen.ast.Struct(member_trail.mark(), member_struct_name, struct_members.unwrap());
			members->append(astgen.ast.EnumMemberStruct(member_trail.mark(), as_struct));
		}

		tok = astgen.cursor.peek();
	}

	consume_tok = astgen.cursor.consume(TokenType::close_curly);

	return astgen.ast.Enum(trail.mark(), struct_name, members);
}
