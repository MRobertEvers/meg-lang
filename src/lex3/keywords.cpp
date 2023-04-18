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
	{"template", TokenKind::TemplateKw},
	{"typename", TokenKind::TypenameKw},
	{"interface", TokenKind::InterfaceKw},
	{"impl", TokenKind::ImplKw},
	{"yield", TokenKind::YieldKw},
	{"async", TokenKind::AsyncKw},
	{"using", TokenKind::UsingKw},
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
