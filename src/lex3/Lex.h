#pragma once

#include "Token.h"

#include <vector>

class Lex
{
	std::vector<char const*> lines_;
	char const* input_;
	int cursor_;
	int input_len_;
	unsigned int curr_line_;

public:
	Lex(char const* input);

	Token next();

private:
	char current();
	char const* head();
	bool is_done();

	TokenKind tok_start();
	Token tok_identifier();
	Token tok_number_literal();

	void consume_whitespace(Token& tok);
};
