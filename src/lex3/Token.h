#pragma once

#include <vector>

enum class TokenKind
{
	// Keyword of other identifier
	Identifier,
	// Float or int
	NumberLiteral,
	// Between ""
	StringLiteral,
	// //
	LineComment,

	OpenParen,
	CloseParen,
	OpenCurly,
	CloseCurly,

	// :
	Colon,
	SemiColon,

	FnKw,
	ReturnKw,

	Bad,

	Eof
};

struct LineMarkers
{
	std::vector<char const*> lines;
	unsigned int num_lines;
};

struct TokenNeighborhood
{
	LineMarkers lines;
	int line_num;
};

struct TokenView
{
	char const* start = nullptr;
	unsigned int size = 0;
	// Trailing newlines

	TokenView();
	TokenView(char const* start);
};

struct Token
{
	TokenNeighborhood neighborhood;
	TokenView view;
	TokenKind kind = TokenKind::Bad;

	unsigned int num_trailing_newlines = 0;

	Token();
	Token(TokenView view, TokenKind kind);
};
