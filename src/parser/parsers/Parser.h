#pragma once

#include "../../ast/ast.h"
#include "../../lexer/TokenCursor.h"

#include <memory>
#include <vector>
using namespace ast;

class Parser
{
public:
	Parser();

	std::unique_ptr<IStatementNode> parse_module_top_level_item(TokenCursor& cursor);

	std::unique_ptr<ast::Module> parse_module(TokenCursor& cursor);

	std::unique_ptr<Let> parse_let(TokenCursor& cursor);
	std::unique_ptr<Block> parse_block(TokenCursor& cursor);
	std::unique_ptr<IStatementNode> parse_struct(TokenCursor& cursor);

	std::unique_ptr<IExpressionNode>
	parse_bin_op(TokenCursor& cursor, int ExprPrec, std::unique_ptr<IExpressionNode> LHS);

	std::unique_ptr<IExpressionNode> parse_literal(TokenCursor& cursor);
	std::unique_ptr<Identifier> parse_identifier(TokenCursor& cursor);

	std::unique_ptr<IExpressionNode> parse_simple_expr(TokenCursor& cursor);
	std::unique_ptr<IExpressionNode> parse_expr(TokenCursor& cursor);

	std::unique_ptr<Block> parse_function_body(TokenCursor& cursor);
	std::unique_ptr<IStatementNode> parse_function(TokenCursor& cursor);
};