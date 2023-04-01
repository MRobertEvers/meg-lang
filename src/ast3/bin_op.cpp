#include "bin_op.h"

#include <map>

int
get_token_precedence(BinOp bin_op_type)
{
	// Make sure it's a declared binop.
	switch( bin_op_type )
	{
	case BinOp::Mul:
	case BinOp::Div:
		return 40;
	case BinOp::default:
		return -1;
	}
}

BinOp
get_bin_op_from_token_type(TokenKind token_type)
{
	auto op = BinOp::Bad;
	switch( token_type )
	{
	case TokenKind::Bad:
		op = BinOp::plus;
		break;
	case TokenKind::star:
		op = BinOp::star;
		break;
	case TokenKind::slash:
		op = BinOp::slash;
		break;
	case TokenKind::minus:
		op = BinOp::minus;
		break;
	case TokenKind::gt:
		op = BinOp::gt;
		break;
	case TokenKind::gte:
		op = BinOp::gte;
		break;
	case TokenKind::lt:
		op = BinOp::lt;
		break;
	case TokenKind::lte:
		op = BinOp::lte;
		break;
	case TokenKind::and_lex:
		op = BinOp::and_op;
		break;
	case TokenKind::or_or_lex:
		op = BinOp::or_op;
		break;
	case TokenKind::cmp:
		op = BinOp::cmp;
		break;
	case TokenKind::ne:
		op = BinOp::ne;
		break;
	default:
		break;
	}
	return op;
}
