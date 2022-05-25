#ifndef TOKEN_H_
#define TOKEN_H_

#include "common/Vec.h"

enum class TokenType
{
	// Keyword or other identifier
	identifier,
	// Int or string
	literal,

	// Comment
	line_comment,

	// One character tokens
	star,
	plus,
	minus,
	slash,
	gt, // Greater Than

	open_paren,
	close_paren,
	open_curly,
	close_curly,

	comma,
	dot,
	semicolon,
	colon,
	equal,

	// Keyword
	fn,
	return_keyword,
	let,
	struct_keyword,
	if_keyword,
	for_keyword,
	else_keyword,
	while_keyword,

	eof,
	bad,
};

enum class LiteralType
{
	integer,
	floating,
	string,
	string_template,
	none
};

struct LineMarkers
{
	Vec<char const*> lines;
	unsigned int num_lines;
};

struct TokenNeighborhood
{
	LineMarkers lines;
	int line_num;
};

struct Token
{
	TokenNeighborhood neighborhood;
	TokenType type = TokenType::bad;
	LiteralType literal_type = LiteralType::none;
	char const* start = nullptr;
	unsigned int size = 0;
	unsigned int num_trailing_newlines = 0;

	Token(){};
	Token(TokenType type)
		: type(type){};
};

#endif