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

	// ...
	var_args,

	// =>
	fat_arrow,
	is,

	exclam,
	mul_equal,
	div_equal,
	sub_equal,
	plus_equal,
	star,
	plus,
	minus,
	slash,
	gt,	 // Greater Than
	gte, // Greater Than equal
	lt,	 // Less Than
	lte, // Less Than equal
	ne,	 // Not equal
	cmp, // ==
	and_lex,
	and_and_lex,
	or_or_lex,

	open_paren,
	close_paren,
	open_curly,
	close_curly,
	open_square,
	close_square,

	indirect_member_access, // ->

	comma,
	dot,
	semicolon,
	colon,
	colon_colon,
	equal,

	// Keyword
	fn,
	switch_keyword,
	case_keyword,
	extern_keyword,
	return_keyword,
	let,
	struct_keyword,
	enum_keyword,
	union_keyword,
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