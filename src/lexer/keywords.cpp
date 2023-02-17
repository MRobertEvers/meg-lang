#include "keywords.h"

#include <cstring>

struct KeywordIdentifierTuple
{
	char const* keyword;
	TokenType type;
};

// clang-format off
static
KeywordIdentifierTuple keywords[] =
{
	// clang-format on
	{"fn", TokenType::fn},
	{"extern", TokenType::extern_keyword},
	{"return", TokenType::return_keyword},
	{"let", TokenType::let},
	{"struct", TokenType::struct_keyword},
	{"union", TokenType::union_keyword},
	{"if", TokenType::if_keyword},
	{"else", TokenType::else_keyword},
	{"while", TokenType::while_keyword},
	{"for", TokenType::for_keyword},
	{"...", TokenType::var_args},
	// clang-format off
};
// clang-format on

TokenType
get_identifier_or_keyword_type(Token const& token)
{
	for( auto& keyword : keywords )
	{
		if( strncmp(keyword.keyword, token.start, strlen(keyword.keyword)) == 0 )
		{
			return keyword.type;
		}
	}

	return TokenType::identifier;
}

struct DebugTokenTypeString
{
	TokenType type;
	char const* typestring;
};

static DebugTokenTypeString debug_tokentype_string[] = {
	{TokenType::identifier, "identifier"},
	{TokenType::literal, "literal"},
	{TokenType::star, "star"},
	{TokenType::exclam, "exclam"},
	{TokenType::mul_equal, "mul_equal"},
	{TokenType::div_equal, "div_equal"},
	{TokenType::plus_equal, "plus_equal"},
	{TokenType::sub_equal, "sub_equal"},
	{TokenType::gt, "gt"},
	{TokenType::gte, "gte"},
	{TokenType::lt, "lt"},
	{TokenType::lte, "lte"},
	{TokenType::ne, "!="},
	{TokenType::cmp, "=="},
	{TokenType::and_lex, "&"},
	{TokenType::and_and_lex, "&&"},
	{TokenType::or_or_lex, "||"},
	{TokenType::plus, "plus"},
	{TokenType::minus, "minus"},
	{TokenType::slash, "slash"},
	{TokenType::line_comment, "comment"},
	{TokenType::if_keyword, "if"},
	{TokenType::else_keyword, "else"},
	{TokenType::open_paren, "open_paren"},
	{TokenType::close_paren, "close_paren"},
	{TokenType::open_curly, "open_curly"},
	{TokenType::close_curly, "close_curly"},
	{TokenType::semicolon, "semicolon"},
	{TokenType::colon, "colon"},
	{TokenType::comma, "comma"},
	{TokenType::fn, "fn"},
	{TokenType::extern_keyword, "extern"},
	{TokenType::return_keyword, "return"},
	{TokenType::while_keyword, "while"},
	{TokenType::let, "let"},
	{TokenType::equal, "equal"},
	{TokenType::struct_keyword, "struct"},
	{TokenType::union_keyword, "union"},
	{TokenType::dot, "dot"},
	{TokenType::eof, "<EOF>"},
	{TokenType::bad, "bad"},
	{TokenType::for_keyword, "for"},
	{TokenType::var_args, "..."},
	{TokenType::indirect_member_access, "indirect_member_access"},
};

char const*
get_tokentype_string(Token const& token)
{
	for( auto& keyword : debug_tokentype_string )
	{
		if( token.type == keyword.type )
		{
			return keyword.typestring;
		}
	}

	return "unk";
}