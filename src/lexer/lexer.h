#ifndef LEXER_H_
#define LEXER_H_

#include "token.h"

#include <cstring>
#include <vector>

class Lexer
{
public:
	Lexer(char const* in)
		: input_(in)
		, cursor_(0)
		, input_len_(strlen(in))
	{}

	std::vector<Token> lex();

private:
	char const* input_;
	int cursor_;
	int input_len_;

	Token lex_consume_identifier();
	Token lex_consume_number();
	Token lex_consume_single();

public:
	static void print_tokens(std::vector<Token> const& tokens);
};

#endif