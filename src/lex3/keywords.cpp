#include "keywords.h"

#include <cstring>

struct KeywordIdentifierTuple
{
	char const* keyword;
	TokenKind type;
};

// clang-format off
static
KeywordIdentifierTuple keywords[] =
{
	// clang-format on
	{"fn", TokenKind::FnKw},
	{"return", TokenKind::ReturnKw},
	// clang-format off
};
// clang-format on

TokenKind
get_identifier_or_keyword_type(TokenView const& token)
{
	for( auto& keyword : keywords )
	{
		auto kw_len = strlen(keyword.keyword);

		if( kw_len == token.size && strncmp(keyword.keyword, token.start, kw_len) == 0 )
			return keyword.type;
	}

	return TokenKind::Identifier;
}

char const*
get_tokentype_string(TokenKind const& tok_kind)
{
	switch( tok_kind )
	{
	case TokenKind::Identifier:
		return "Identifier";
	case TokenKind::NumberLiteral:
		return "NumberLiteral";
	case TokenKind::StringLiteral:
		return "StringLiteral";
	case TokenKind::LineComment:
		return "LineComment";
	case TokenKind::OpenParen:
		return "OpenParen";
	case TokenKind::CloseParen:
		return "CloseParen";
	case TokenKind::OpenCurly:
		return "OpenCurly";
	case TokenKind::CloseCurly:
		return "CloseCurly";
	case TokenKind::Colon:
		return "Colon";
	case TokenKind::SemiColon:
		return "SemiColon";
	case TokenKind::Star:
		return "Star";
	case TokenKind::Slash:
		return "Slash";
	case TokenKind::Plus:
		return "Plus";
	case TokenKind::Minus:
		return "Minus";
	case TokenKind::FnKw:
		return "FnKw";
	case TokenKind::ReturnKw:
		return "ReturnKw";
	case TokenKind::Bad:
		return "Bad";
	case TokenKind::Eof:
		return "Eof";
	}

	return "Unk";
}