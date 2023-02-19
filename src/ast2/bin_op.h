
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
	and_op,
	or_op,
	cmp,
	is,
	ne,
	bad
};

int get_token_precedence(BinOp bin_op_type);

BinOp get_bin_op_from_token_type(TokenType token_type);

void init_bin_op_lookup();

} // namespace ast
