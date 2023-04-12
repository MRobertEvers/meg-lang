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
		return 320;
	case BinOp::Add:
	case BinOp::Sub:
		return 160;
	case BinOp::Is:
		return 80;
	case BinOp::And:
		return 40;
	case BinOp::Or:
		return 20;
	case BinOp::Eq:
	case BinOp::Gt:
	case BinOp::Gte:
	case BinOp::Lte:
	case BinOp::Lt:
		return 10;
	case BinOp::Assign:
		return 5;
	default:
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
		op = BinOp::Bad;
		break;
	case TokenKind::Star:
		op = BinOp::Mul;
		break;
	case TokenKind::Slash:
		op = BinOp::Div;
		break;
	case TokenKind::Minus:
		op = BinOp::Sub;
		break;
	case TokenKind::Plus:
		op = BinOp::Add;
		break;
	case TokenKind::ExclamEq:
		op = BinOp::Neq;
		break;
	case TokenKind::EqEq:
		op = BinOp::Eq;
		break;
	case TokenKind::Lt:
		op = BinOp::Lt;
		break;
	case TokenKind::Gt:
		op = BinOp::Gt;
		break;
	case TokenKind::LtEq:
		op = BinOp::Lte;
		break;
	case TokenKind::GtEq:
		op = BinOp::Gte;
		break;
	case TokenKind::IsKw:
		op = BinOp::Is;
		break;
	default:
		break;
	}
	return op;
}
