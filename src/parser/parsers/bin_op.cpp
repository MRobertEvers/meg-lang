#include "bin_op.h"

#include <map>

using namespace ast;

// TODO: Dont do this
static std::map<char, int> BinopPrecedence;

int
get_token_precedence(BinOp bin_op_type)
{
	// Make sure it's a declared binop.
	int prec = BinopPrecedence[bin_op_type];
	if( prec <= 0 )
		return -1;
	return prec;
}

ast::BinOp
get_bin_op_from_token_type(TokenType token_type)
{
	auto op = BinOp::bad;
	switch( token_type )
	{
	case TokenType::plus:
		op = BinOp::plus;
		break;
	case TokenType::star:
		op = BinOp::star;
		break;
	case TokenType::slash:
		op = BinOp::slash;
		break;
	case TokenType::minus:
		op = BinOp::minus;
		break;
	case TokenType::gt:
		op = BinOp::gt;
		break;
	case TokenType::gte:
		op = BinOp::gte;
		break;
	case TokenType::lt:
		op = BinOp::lt;
		break;
	case TokenType::lte:
		op = BinOp::lte;
		break;
	case TokenType::and_lex:
		op = BinOp::and_lex;
		break;
	case TokenType::or_lex:
		op = BinOp::or_lex;
		break;
	case TokenType::cmp:
		op = BinOp::cmp;
		break;
	case TokenType::ne:
		op = BinOp::ne;
		break;
	default:
		break;
	}
	return op;
}

void
init_bin_op_lookup()
{
	BinopPrecedence[BinOp::ne] = 10;
	BinopPrecedence[BinOp::cmp] = 10;
	BinopPrecedence[BinOp::or_lex] = 10;
	BinopPrecedence[BinOp::and_lex] = 10;
	BinopPrecedence[BinOp::lte] = 10;
	BinopPrecedence[BinOp::gte] = 10;
	BinopPrecedence[BinOp::lt] = 10;
	BinopPrecedence[BinOp::gt] = 10;
	BinopPrecedence[BinOp::plus] = 20;
	BinopPrecedence[BinOp::minus] = 20;
	BinopPrecedence[BinOp::slash] = 40; // highest.
	BinopPrecedence[BinOp::star] = 40;	// highest.
}