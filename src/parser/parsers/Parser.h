#pragma once

#include "../../ast/ast.h"
#include "../../lexer/TokenCursor.h"

#include <memory>

using namespace ast;

class Parser
{
public:
	Parser();

	std::unique_ptr<IStatementNode> parse_module_top_level_item(TokenCursor& cursor);

	std::unique_ptr<IStatementNode> parse_module(TokenCursor& cursor);

	std::unique_ptr<IStatementNode> parse_block(TokenCursor& cursor);

	std::unique_ptr<IExpressionNode>
	parse_bin_op(TokenCursor& cursor, int ExprPrec, std::unique_ptr<IExpressionNode> LHS);

	std::unique_ptr<IExpressionNode> parse_expression_value(TokenCursor& cursor);

	std::unique_ptr<IExpressionNode> parse_expression(TokenCursor& cursor);

	std::unique_ptr<Prototype> parse_function_proto(TokenCursor& cursor);

	std::unique_ptr<IStatementNode> parse_function_body(TokenCursor& cursor);
	std::unique_ptr<IStatementNode> parse_function(TokenCursor& cursor);
};