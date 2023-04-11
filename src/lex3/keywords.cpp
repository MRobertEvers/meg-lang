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
	{"let", TokenKind::LetKw},
	{"if", TokenKind::IfKw},
	{"else", TokenKind::ElseKw},
	{"true", TokenKind::TrueKw},
	{"false", TokenKind::FalseKw}, //
	{"struct", TokenKind::StructKw},
	{"union", TokenKind::UnionKw},
	{"enum", TokenKind::EnumKw},
	{"sizeof", TokenKind::SizeOfKw},
	{"is", TokenKind::IsKw},
	{"for", TokenKind::ForKw},
	{"while", TokenKind::WhileKw},
	{"extern", TokenKind::ExternKw},
	{"break", TokenKind::BreakKw},
	{"default", TokenKind::DefaultKw},
	{"continue", TokenKind::ContinueKw},
	{"case", TokenKind::CaseKw},
	{"switch", TokenKind::SwitchKw},
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
	case TokenKind::OpenSquare:
		return "OpenSquare";
	case TokenKind::CloseSquare:
		return "CloseSquare";
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
	case TokenKind::Eq:
		return "Eq";
	case TokenKind::FnKw:
		return "FnKw";
	case TokenKind::ReturnKw:
		return "ReturnKw";
	case TokenKind::LetKw:
		return "LetKw";
	case TokenKind::IfKw:
		return "IfKw";
	case TokenKind::ElseKw:
		return "ElseKw";
	case TokenKind::Gt:
		return "Gt";
	case TokenKind::Lt:
		return "Lt";
	case TokenKind::GtEq:
		return "GtEq";
	case TokenKind::LtEq:
		return "LtEq";
	case TokenKind::Bad:
		return "Bad";
	case TokenKind::Comma:
		return "Comma";
	case TokenKind::ColonColon:
		return "ColonColon";
	case TokenKind::TrueKw:
		return "TrueKw";
	case TokenKind::FalseKw:
		return "FalseKw";
	case TokenKind::StructKw:
		return "StructKw";
	case TokenKind::UnionKw:
		return "UnionKw";
	case TokenKind::EnumKw:
		return "EnumKw";
	case TokenKind::SizeOfKw:
		return "SizeOf";
	case TokenKind::EqEq:
		return "EqEq";
	case TokenKind::Ampersand:
		return "Ampersand";
	case TokenKind::Dot:
		return "Dot";
	case TokenKind::IsKw:
		return "IsKw";
	case TokenKind::ForKw:
		return "ForKw";
	case TokenKind::WhileKw:
		return "WhileKw";
	case TokenKind::ExternKw:
		return "ExternKw";
	case TokenKind::BreakKw:
		return "BreakKw";
	case TokenKind::DefaultKw:
		return "DefaultKw";
	case TokenKind::ContinueKw:
		return "ContinueKw";
	case TokenKind::CaseKw:
		return "CaseKw";
	case TokenKind::SkinnyArrow:
		return "SkinnyArrow";
	case TokenKind::FatArrow:
		return "FatArrow";
	case TokenKind::Exclam:
		return "Exclam";
	case TokenKind::ExclamEq:
		return "ExclamEq";
	case TokenKind::SwitchKw:
		return "Switch";
	case TokenKind::Eof:
		return "Eof";
	}

	return "Unk";
}