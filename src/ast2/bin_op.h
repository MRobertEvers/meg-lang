
#pragma once
#include "lexer/token.h"

namespace ast
{

enum BinOp : char
{
	plus,
	star,
	minus,
	slash,
	gt,
	gte,
	lt,
	lte,
	and_lex,
	or_lex,
	cmp,
	ne,
	bad
};

int get_token_precedence(BinOp bin_op_type);

BinOp get_bin_op_from_token_type(TokenType token_type);

void init_bin_op_lookup();

} // namespace ast
