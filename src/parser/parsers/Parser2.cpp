#include "Parser2.h"

#include "common/Vec.h"

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
