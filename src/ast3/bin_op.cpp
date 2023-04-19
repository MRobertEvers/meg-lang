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
	case BinOp::Remainder:
		return 40;
	case BinOp::Add:
	case BinOp::Sub:
		return 35;
	case BinOp::BitShiftDown:
	case BinOp::BitShiftUp:
	case BinOp::Is:
		return 30;
	case BinOp::Gt:
	case BinOp::Gte:
	case BinOp::Lt:
	case BinOp::Lte:
		return 25;
	case BinOp::Eq:
	case BinOp::Neq:
		return 20;
	case BinOp::BitwiseAnd:
		return 16;
	case BinOp::BitwiseXor:
		return 15;
	case BinOp::BitwiseOr:
		return 14;
	case BinOp::And:
		return 11;
	case BinOp::Or:
		return 10;
	case BinOp::Assign:
		return 5;
	case BinOp::Bad:
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
	case TokenKind::AmpAmp:
		op = BinOp::And;
		break;
	case TokenKind::PipePipe:
		op = BinOp::Or;
		break;
	case TokenKind::LtLt:
		op = BinOp::BitShiftUp;
		break;
	case TokenKind::GtGt:
		op = BinOp::BitShiftDown;
		break;
	case TokenKind::Pipe:
		op = BinOp::BitwiseOr;
		break;
	case TokenKind::Ampersand:
		op = BinOp::BitwiseAnd;
		break;
	case TokenKind::Caret:
		op = BinOp::BitwiseXor;
		break;
	case TokenKind::Percent:
		op = BinOp::Remainder;
		break;
	default:
		break;
	}
	return op;
}
