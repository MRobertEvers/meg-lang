#ifndef LEXER_H_
#define LEXER_H_

#include "token.h"

#include <cstring>
#include <vector>

class LexResult
{
public:
	LineMarkers markers;
	std::vector<Token> tokens;

	LexResult(unsigned int num_lines, Vec<char const*> lines, Vec<Token> tokens);
};

class Lexer
{
public:
	Lexer(char const* in)
		: input_(in)
		, cursor_(0)
		, curr_line_(0)
		, input_len_(strlen(in))
	{}

	LexResult lex();

private:
	std::vector<char const*> lines_;
	char const* input_;
	int cursor_;
	int input_len_;
	unsigned int curr_line_;

	Token lex_consume_identifier();
	Token lex_consume_number();
	Token lex_consume_single();
	Token lex_consume_line_comment();
	Token lex_consume_ambiguous_lexeme();
	Token lex_consume_string_literal();

	void track_newline(std::vector<Token>& tokens);

	bool peek(char const* seq);

	Token new_token(TokenType token_type, int size);

public:
	static void print_tokens(std::vector<Token> const& tokens);
};

#endif