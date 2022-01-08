#include "Parser2.h"

#include "common/Vec.h"
#include "common/unreachable.h"

ParseResult<ast::Module>
Parser2::parse_module()
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
Parser2::parse_module_top_level_item()
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

ParseResult<Let>
Parser2::parse_let()
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

	tok = cursor.consume(TokenType::colon, TokenType::equal);
	if( !tok.ok() )
	{
		return ParseError("Expected identifier or '='", tok.as());
	}

	switch( tok.unwrap().type )
	{
	case TokenType::colon:
		/* code */
		break;

	case TokenType::equal:
		/* code */
		break;

	default:
		unreachable();
		break;
	}

	auto type = std::make_unique<TypeIdentifier>(TypeIdentifier::Empty());
	if( tok.type == TokenType::colon )
	{
		cursor.adv();
		tok = cursor.peek();
		if( tok.type != TokenType::identifier )
		{
			std::cout << "Expected type identifier";
			return nullptr;
		}

		type = std::make_unique<TypeIdentifier>(std::string{tok.start, tok.size});
		cursor.adv();
		tok = cursor.peek();
		while( tok.type == TokenType::star )
		{
			type = std::make_unique<TypeIdentifier>(std::move(type));
			cursor.adv();
			tok = cursor.peek();
		}
	}

	if( tok.type != TokenType::equal )
	{
		std::cout << "Expected assignment '='";
		return nullptr;
	}
	cursor.adv();

	auto expr = parse_expr(cursor);

	return std::make_unique<Let>(
		std::make_unique<ValueIdentifier>(std::string{id_tok.start, id_tok.size}),
		std::make_unique<TypeIdentifier>(std::move(type)),
		std::move(expr));
	return ParseError("Not implemented.");
}

ParseResult<Block>
Parser2::parse_block()
{
	return ParseError("Not implemented.");
}

ParseResult<IStatementNode>
Parser2::parse_struct()
{
	return ParseError("Not implemented.");
}

ParseResult<IExpressionNode>
Parser2::parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS)
{
	auto p = ParseResult<IExpressionNode>{ParseError("Not implemented.")};
	return p;
}

ParseResult<IExpressionNode>
Parser2::parse_literal()
{
	return ParseError("Not implemented.");
}

ParseResult<Identifier>
Parser2::parse_identifier()
{
	return ParseError("Not implemented.");
}

ParseResult<IExpressionNode>
Parser2::parse_simple_expr()
{
	return ParseError("Not implemented.");
}

ParseResult<IExpressionNode>
Parser2::parse_expr()
{
	return ParseError("Not implemented.");
}

ParseResult<IStatementNode>
Parser2::parse_function()
{
	return ParseError("Not implemented.");
}

ParseResult<Block>
Parser2::parse_function_body()
{
	auto p = ParseResult<Block>{ParseError("Not implemented.")};
	return p;
}
