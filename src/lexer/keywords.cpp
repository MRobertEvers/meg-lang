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
	{"return", TokenType::return_keyword},
	{"let", TokenType::let},
	{"struct", TokenType::struct_keyword},
	{"if", TokenType::if_keyword},
	{"else", TokenType::else_keyword}
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
	{TokenType::plus, "plus"},
	{TokenType::minus, "minus"},
	{TokenType::slash, "slash"},
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
	{TokenType::return_keyword, "return_keyword"},
	{TokenType::let, "let"},
	{TokenType::equal, "equal"},
	{TokenType::struct_keyword, "struct"},
	{TokenType::dot, "dot"},
	{TokenType::eof, "<EOF>"},
	{TokenType::bad, "bad"},
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