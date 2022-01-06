#include "Parser.h"

#include <iostream>
#include <map>
#include <memory>

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

std::unique_ptr<ast::Module>
Parser::parse_module(TokenCursor& cursor)
{
	std::vector<std::unique_ptr<IStatementNode>> nodes;

	while( cursor.has_tokens() )
	{
		auto item = Parser::parse_module_top_level_item(cursor);
		if( item == nullptr )
		{
			break;
		}
		nodes.emplace_back(std::move(item));
	}
	return std::make_unique<ast::Module>(std::move(nodes));
}

std::unique_ptr<Let>
Parser::parse_let(TokenCursor& cursor)
{
	auto tok = cursor.peek();
	if( tok.type != TokenType::let )
	{
		std::cout << "Expected 'let'" << std::endl;
		return nullptr;
	}
	cursor.adv();

	auto id_tok = cursor.peek();
	if( id_tok.type != TokenType::identifier )
	{
		std::cout << "Expected identifier" << std::endl;
		return nullptr;
	}
	cursor.adv();

	tok = cursor.peek();
	if( tok.type != TokenType::colon && tok.type != TokenType::equal )
	{
		std::cout << "Expected type identifier or '='";
		return nullptr;
	}

	Identifier type = Identifier::Empty();
	if( tok.type == TokenType::colon )
	{
		cursor.adv();
		tok = cursor.peek();
		if( tok.type != TokenType::identifier )
		{
			std::cout << "Expected type identifier";
			return nullptr;
		}

		type = Identifier{std::string{tok.start, tok.size}};
		cursor.adv();
		tok = cursor.peek();
	}

	if( tok.type != TokenType::equal )
	{
		std::cout << "Expected assignment '='";
		return nullptr;
	}
	cursor.adv();

	auto expr = parse_expr(cursor);

	return std::make_unique<Let>(
		Identifier{std::string{id_tok.start, id_tok.size}}, type, std::move(expr));
}

std::unique_ptr<Block>
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
		// TODO: parse statement (i.e. this consumes the semicolon)
		switch( tok.type )
		{
		case TokenType::return_keyword:
			cursor.adv();
			stmts.push_back(
				std::move(std::make_unique<Return>(std::move(Parser::parse_expr(cursor)))));
			break;
		case TokenType::let:
			stmts.emplace_back(parse_let(cursor));
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
	cursor.adv();

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
		auto RHS = Parser::parse_simple_expr(cursor);
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
Parser::parse_literal(TokenCursor& cursor)
{
	auto tok = cursor.peek();
	if( tok.type != TokenType::literal )
	{
		std::cout << "Expected literal" << std::endl;
		return nullptr;
	}

	cursor.adv();

	switch( tok.literal_type )
	{
	case LiteralType::integer:
	{
		auto sz = std::string{tok.start, tok.size};
		int val = std::stoi(sz);
		return std::make_unique<Number>(val);
	}
	break;

	default:
		std::cout << "Unsupported literal type" << std::endl;
		return nullptr;
		break;
	}
}

std::unique_ptr<IExpressionNode>
Parser::parse_simple_expr(TokenCursor& cursor)
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::literal:
		return parse_literal(cursor);

	case TokenType::identifier:

	{
		auto id = std::make_unique<Identifier>(std::string{tok.start, tok.size});
		cursor.adv();
		return id;
	}
	break;

	default:
		std::cout << "Unexpected token while parsing expression" << std::endl;
		break;
	}
	return nullptr;
}

std::unique_ptr<IExpressionNode>
Parser::parse_expr(TokenCursor& cursor)
{
	auto LHS = parse_simple_expr(cursor);
	if( !LHS )
	{
		return nullptr;
	}

	auto OP = Parser::parse_bin_op(cursor, 0, std::move(LHS));

	return OP;
}

static std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>>
parse_function_parameter_list(TokenCursor& cursor)
{
	std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>> result;

	Token curr_tok = cursor.peek();

	while( curr_tok.type != TokenType::close_paren )
	{
		if( curr_tok.type != TokenType::identifier )
		{
			// TODO: Error type.
			return result;
		}

		auto name_tok = curr_tok;

		cursor.adv();
		curr_tok = cursor.peek();
		if( curr_tok.type != TokenType::colon )
		{
			// TODO: Error type.
			return result;
		}

		cursor.adv();
		curr_tok = cursor.peek();
		if( curr_tok.type != TokenType::identifier )
		{
			// TODO: Error type.
			return result;
		}

		auto type_tok = curr_tok;
		result.push_back(std::move(std::make_unique<std::pair<Identifier, Identifier>>(
			Identifier{std::string{name_tok.start, name_tok.size}},
			Identifier{std::string{type_tok.start, type_tok.size}})));

		cursor.adv();
		curr_tok = cursor.peek();

		// Also catches trailing comma.
		if( curr_tok.type == TokenType::comma )
		{
			cursor.adv();
			curr_tok = cursor.peek();
		}
	}

	return result;
}

static std::unique_ptr<Prototype>
parse_function_proto(TokenCursor& cursor)
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

	cursor.adv();
	std::vector<std::unique_ptr<std::pair<Identifier, Identifier>>> params =
		parse_function_parameter_list(cursor);

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
	return std::unique_ptr<Prototype>{new Prototype{
		Identifier{std::string{tok_fn_name.start, tok_fn_name.size}},
		Identifier{std::string{tok_fn_return_type.start, tok_fn_return_type.size}},
		params}};
}

std::unique_ptr<Block>
Parser::parse_function_body(TokenCursor& cursor)
{
	return Parser::parse_block(cursor);
}

std::unique_ptr<IStatementNode>
Parser::parse_function(TokenCursor& cursor)
{
	auto proto = parse_function_proto(cursor);
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

	return std::make_unique<ast::Function>(std::move(proto), std::move(definition));
}
