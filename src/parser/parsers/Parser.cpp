#include "Parser.h"

#include "common/Vec.h"
#include "common/unreachable.h"

#include <map>

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

Parser::Parser(TokenCursor& cursor)
	: cursor(cursor)
{
	BinopPrecedence['<'] = 10;
	BinopPrecedence['+'] = 20;
	BinopPrecedence['-'] = 20;
	BinopPrecedence['*'] = 40; // highest.
}

ParseResult<ast::Module>
Parser::parse_module()
{
	Vec<OwnPtr<IStatementNode>> nodes;

	while( cursor.has_tokens() )
	{
		auto item = parse_module_top_level_item();
		if( !item.ok() )
		{
			return item;
		}
		nodes.emplace_back(item.unwrap());
	}

	return OwnPtr<ast::Module>::of(std::move(nodes));
}

ParseResult<IStatementNode>
Parser::parse_module_top_level_item()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::fn:
	{
		return parse_function();
	}
	case TokenType::struct_keyword:
		return parse_struct();
	default:
		return ParseError("Expected top level 'fn' or 'struct' declaration.");
	}
}

static ValueIdentifier
to_value_identifier(ConsumeResult const& tok_res)
{
	auto tok = tok_res.unwrap();

	return ValueIdentifier{String{tok.start, tok.size}};
}

ParseResult<Let>
Parser::parse_let()
{
	auto tok = cursor.consume(TokenType::let);
	if( !tok.ok() )
	{
		return ParseError("Expected 'let'", tok.as());
	}

	tok = cursor.consume(TokenType::identifier);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier", tok.as());
	}
	auto name = OwnPtr<ValueIdentifier>::of(to_value_identifier(tok));

	tok = cursor.consume(TokenType::colon, TokenType::equal);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier or '='", tok.as());
	}

	auto type = parse_type_decl(true);
	if( !type.ok() )
	{
		return type;
	}

	cursor.consume_if_expected(TokenType::equal);
	auto expr = parse_expr();
	if( !expr.ok() )
	{
		return expr;
	}

	return ast::Let{std::move(name), type.unwrap(), expr.unwrap()};
}

ParseResult<Block>
Parser::parse_block()
{
	return ParseError("Not implemented.");
}

static TypeIdentifier
to_type_identifier(ConsumeResult const& tok_res)
{
	auto tok = tok_res.unwrap();

	return TypeIdentifier{String{tok.start, tok.size}};
}

ParseResult<TypeIdentifier>
Parser::parse_type_decl(bool allow_empty)
{
	auto tok = cursor.consume_if_expected(TokenType::identifier);
	if( !tok.ok() )
	{
		if( allow_empty )
		{
			return TypeIdentifier::Empty();
		}
		else
		{
			return ParseError("Unexpected token while parsing type.", tok.as());
		}
	}

	auto type = OwnPtr<TypeIdentifier>::of(to_type_identifier(tok));

	auto star_tok = cursor.consume_if_expected(TokenType::star);
	while( star_tok.ok() )
	{
		type = TypeIdentifier::PointerToTy(std::move(type));
		star_tok = cursor.consume_if_expected(TokenType::star);
	}

	return std::move(type);
}

ParseResult<Struct>
Parser::parse_struct()
{
	Vec<OwnPtr<ast::MemberVariableDeclaration>> members;

	auto consume_tok = cursor.consume(TokenType::struct_keyword);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected 'struct'", consume_tok.as());
	}

	consume_tok = cursor.consume(TokenType::identifier);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected struct identifier.", consume_tok.as());
	}

	auto struct_name = OwnPtr<TypeIdentifier>::of(to_type_identifier(consume_tok));

	consume_tok = cursor.consume(TokenType::open_curly);
	if( !consume_tok.ok() )
	{
		return ParseError("Expected block, '{'.", consume_tok.as());
	}

	auto tok = cursor.peek();
	while( tok.type != TokenType::close_curly )
	{
		consume_tok = cursor.consume(TokenType::identifier);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected member declaration.", consume_tok.as());
		}

		auto name = OwnPtr<ValueIdentifier>::of(to_value_identifier(consume_tok));

		consume_tok = cursor.consume(TokenType::colon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ':'.", consume_tok.as());
		}

		auto decl = parse_type_decl(false);
		if( !decl.ok() )
		{
			return decl;
		}

		consume_tok = cursor.consume(TokenType::semicolon);
		if( !consume_tok.ok() )
		{
			return ParseError("Expected ';'.", consume_tok.as());
		}

		members.emplace_back(new ast::MemberVariableDeclaration{std::move(name), decl.unwrap()});

		tok = cursor.peek();
	}

	return Struct{std::move(struct_name), std::move(members)};
}

ParseResult<IExpressionNode>
Parser::parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS)
{
	// If this is a binop, find its precedence.
	while( true )
	{
		auto cur = cursor.peek();
		int TokPrec = get_token_precedence(cur);

		// If this is a binop that binds at least as tightly as the current binop,
		// consume it, otherwise we are done.
		if( TokPrec < ExprPrec )
			return std::move(LHS);

		// This is a binary operation because TokPrec would be less than ExprPrec if
		// the next token was not a bin op (e.g. if statement or so.)
		char Op = *cur.start;

		// TODO: Consume bin op
		cursor.consume(TokenType::struct_keyword);

		// Parse the primary expression after the binary operator.
		auto RHS = parse_simple_expr();
		if( !RHS.ok() )
			return RHS;

		// If BinOp binds less tightly with RHS than the operator after RHS, let
		// the pending operator take RHS as its LHS.
		cur = cursor.peek();
		int NextPrec = get_token_precedence(cur);
		if( TokPrec < NextPrec )
		{
			RHS = parse_bin_op(TokPrec + 1, RHS.unwrap());
			if( !RHS.ok() )
				return RHS;
		}

		// Merge LHS/RHS.
		LHS = OwnPtr<IExpressionNode>(new BinaryOperation{Op, std::move(LHS), RHS.unwrap()});
	}
}

ParseResult<IExpressionNode>
Parser::parse_literal()
{
	return ParseError("Not implemented.");
}

ParseResult<Identifier>
Parser::parse_identifier()
{
	// std::vector<Token> toks;

	// auto tok = cursor.peek();
	// if( tok.type != TokenType::identifier )
	// {
	// 	std::cout << "Expected identifier" << std::endl;
	// 	return nullptr;
	// }
	// cursor.adv();

	// toks.push_back(tok);
	// tok = cursor.peek();
	// while( tok.type == TokenType::dot )
	// {
	// 	cursor.adv();
	// 	tok = cursor.peek();

	// 	if( tok.type != TokenType::identifier )
	// 	{
	// 		std::cout << "Expected identifier" << std::endl;
	// 		return nullptr;
	// 	}
	// 	toks.push_back(tok);
	// 	cursor.adv();
	// }

	// std::vector<std::string> path;
	// for( auto& t : toks )
	// {
	// 	path.emplace_back(t.start, t.size);
	// }

	// return std::make_unique<ValueIdentifier>(path);
	return ParseError("Not implemented.");
}

ParseResult<IExpressionNode>
Parser::parse_simple_expr()
{
	auto tok = cursor.peek();
	switch( tok.type )
	{
	case TokenType::literal:
		return parse_literal();

	case TokenType::identifier:
		return parse_identifier();

	default:
		return ParseError("Expected simple expression.");
	}
}

ParseResult<IExpressionNode>
Parser::parse_expr()
{
	return ParseError("Not implemented.");
}

ParseResult<Function>
Parser::parse_function()
{
	return ParseError("Not implemented.");
}

ParseResult<Block>
Parser::parse_function_body()
{
	auto p = ParseResult<Block>{ParseError("Not implemented.")};
	return p;
}
