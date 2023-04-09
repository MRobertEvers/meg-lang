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

	Comma,
	OpenSquare,
	CloseSquare,
	OpenParen,
	CloseParen,
	OpenCurly,
	CloseCurly,

	Ampersand,
	SkinnyArrow,
	FatArrow,
	Dot,
	Colon,
	ColonColon,
	SemiColon,

	Star,
	Slash,
	Plus,
	Minus,
	EqEq,
	Eq,
	Gt,
	Lt,
	GtEq,
	LtEq,

	FnKw,
	IfKw,
	TrueKw,
	FalseKw,
	ElseKw,
	ReturnKw,
	LetKw,
	StructKw,
	UnionKw,
	EnumKw,
	SizeOfKw,
	IsKw,
	ForKw,
	WhileKw,
	ExternKw,
	BreakKw,
	ContinueKw,
	DefaultKw,
	CaseKw,

	Bad,

	Eof
};

struct LineMarkers
{
	std::vector<char const*> lines;
	unsigned int num_lines = 0;
};

struct TokenNeighborhood
{
	LineMarkers lines;
	int line_num = 0;
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
