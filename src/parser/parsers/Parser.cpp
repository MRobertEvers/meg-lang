#include "Parser.h"

#include "../parser.h"

#include <iostream>

// TODO: Dont do this
static std::map<char, int> BinopPrecedence;

static int
get_token_precedence(Token const& token)
{
	// Make sure it's a declared binop.
	int TokPrec = BinopPrecedence[*token.start];
	if( TokPrec <= 0 )
		return -1;
	return TokPrec;
}

Parser::Parser()
{
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.
}

std::unique_ptr<IStatementNode>
Parser::parse_module_top_level_item(TokenCursor& cursor)
{
	switch( cursor.peek().type )
	{
	case TokenType::fn:
	{
		cursor.adv();
		return Parser::parse_function(cursor);
	}
	default:
		std::cout << "Expected function definition" << std::endl;
		break;
	}

	return nullptr;
}

std::unique_ptr<IStatementNode>
Parser::parse_module(TokenCursor& cursor)
{
	return Parser::parse_module_top_level_item(cursor);
}

std::unique_ptr<IStatementNode>
Parser::parse_block(TokenCursor& cursor)
{
	std::vector<std::unique_ptr<IStatementNode>> stmts;

	auto tok = cursor.peek();
	if( tok.type != TokenType::open_curly )
	{
		std::cout << "Expected '{'" << std::endl;
		return nullptr;
	}

	cursor.adv();
	tok = cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		switch( tok.type )
		{
		case TokenType::return_keyword:
			cursor.adv();
			stmts.push_back(
				std::move(std::make_unique<Return>(std::move(Parser::parse_expression(cursor)))));
			break;
		default:
			std::cout << "Expected expression or return statement" << std::endl;
			break;
		}

		tok = cursor.peek();
		if( tok.type != TokenType::semicolon )
		{
			std::cout << "Expected ';'" << std::endl;
			return nullptr;
		}

		cursor.adv();
		tok = cursor.peek();
	}

	return std::make_unique<Block>(std::move(stmts));
}

std::unique_ptr<IExpressionNode>
Parser::parse_bin_op(TokenCursor& cursor, int ExprPrec, std::unique_ptr<IExpressionNode> LHS)
{
	// If this is a binop, find its precedence.
	while( true )
	{
		auto cur = cursor.peek();
		int TokPrec = get_token_precedence(cur);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( TokPrec < ExprPrec )
			return LHS;

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)
		char Op = *cursor.peek().start;
		cursor.adv();

		// Parse the primary expression after the binary operator.
		auto RHS = Parser::parse_expression(cursor);
		if( !RHS )
			return nullptr;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		int NextPrec = get_token_precedence(cursor.peek());
		if( TokPrec < NextPrec )
		{
			RHS = Parser::parse_bin_op(cursor, TokPrec + 1, std::move(RHS));
			if( !RHS )
				return nullptr;
		}

		// Merge LHS/RHS.
		LHS = std::make_unique<BinaryOperation>(Op, std::move(LHS), std::move(RHS));
	}
}

std::unique_ptr<IExpressionNode>
Parser::parse_expression_value(TokenCursor& cursor)
{
	auto tok = cursor.peek();
	if( tok.type != TokenType::literal )
	{
		std::cout << "Expected literal" << std::endl;
		return nullptr;
	}

	auto sz = std::string{tok.start, tok.size};
	int val = std::stoi(sz);
	cursor.adv();
	return std::make_unique<Number>(val);
}

std::unique_ptr<IExpressionNode>
Parser::parse_expression(TokenCursor& cursor)
{
	auto LHS = Parser::parse_expression_value(cursor);
	if( !LHS )
	{
		return nullptr;
	}

	auto OP = Parser::parse_bin_op(cursor, 0, std::move(LHS));

	return OP;
}

std::unique_ptr<Prototype>
Parser::parse_function_proto(TokenCursor& cursor)
{
	Token tok_fn_name = cursor.peek();
	if( tok_fn_name.type != TokenType::identifier )
	{
		std::cout << "Expected function name identifier" << std::endl;
		return nullptr;
	}

	cursor.adv();
	if( cursor.peek().type != TokenType::open_paren )
	{
		std::cout << "Expected '('" << std::endl;
		return nullptr;
	}

	// TODO: Identifiers for args
	cursor.adv();
	if( cursor.peek().type != TokenType::close_paren )
	{
		std::cout << "Expected ')'" << std::endl;
		return nullptr;
	}

	cursor.adv();
	if( cursor.peek().type != TokenType::colon )
	{
		std::cout << "Expected ':'" << std::endl;
		return nullptr;
	}

	cursor.adv();
	Token tok_fn_return_type = cursor.peek();
	if( tok_fn_return_type.type != TokenType::identifier )
	{
		std::cout << "Expected return type" << std::endl;
		return nullptr;
	}

	cursor.adv();
	return std::make_unique<Prototype>(
		std::string{tok_fn_name.start, tok_fn_name.size}, std::vector<std::string>{});
}

std::unique_ptr<IStatementNode>
Parser::parse_function_body(TokenCursor& cursor)
{
	return Parser::parse_block(cursor);
}

std::unique_ptr<IStatementNode>
Parser::parse_function(TokenCursor& cursor)
{
	auto proto = Parser::parse_function_proto(cursor);
	if( !proto )
	{
		return nullptr;
	}

	// TODO: Pass in proto to check return type
	auto definition = Parser::parse_function_body(cursor);
	if( !definition )
	{
		return nullptr;
	}

	return std::make_unique<nodes::Function>(std::move(proto), std::move(definition));
}
