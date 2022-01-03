#ifndef TOKEN_H_
#define TOKEN_H_

enum class TokenType
{
	// Keyword or other identifier
	identifier,
	// Int or string
	literal,

	// One character tokens
	star,
	plus,
	minus,
	slash,

	open_paren,
	close_paren,
	open_curly,
	close_curly,

	semicolon,
	colon,

	// Keyword
	fn,
	return_keyword,

	bad
};

enum class LiteralType
{
	integer,
	floating,
	string,
	string_template,
	none
};

struct Token
{
	TokenType type = TokenType::bad;
	LiteralType literal_type = LiteralType::none;
	char const* start = nullptr;
	unsigned int size = 0;
};

#endif