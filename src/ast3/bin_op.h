
#pragma once
#include "lex3/Token.h"

enum class BinOp
{
	Add,
	Mul,
	Sub,
	Div,
	Gt,
	Gte,
	Lt,
	Lte,
	And,
	Or,
	Remainder,
	BitShiftUp,
	BitShiftDown,
	BitwiseXor,
	BitwiseAnd,
	BitwiseOr,
	Eq,
	Is,
	Assign,
	Neq,
	Bad
};

int get_token_precedence(BinOp bin_op_type);

BinOp get_bin_op_from_token_type(TokenKind token_type);
