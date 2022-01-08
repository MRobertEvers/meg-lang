#pragma once
#include "../../ast/ast.h"
#include "../../lexer/TokenCursor.h"
#include "../ParseResult.h"
#include "common/OwnPtr.h"

using namespace ast;

class Parser2
{
	TokenCursor& cursor;

public:
	Parser2(TokenCursor& cursor)
		: cursor(cursor){};

	ParseResult<ast::Module> parse_module();

private:
	ParseResult<IStatementNode> parse_module_top_level_item();

	ParseResult<Let> parse_let();
	ParseResult<Block> parse_block();
	ParseResult<IStatementNode> parse_struct();

	ParseResult<IExpressionNode> parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS);

	ParseResult<IExpressionNode> parse_literal();
	ParseResult<Identifier> parse_identifier();

	ParseResult<IExpressionNode> parse_simple_expr();
	ParseResult<IExpressionNode> parse_expr();

	ParseResult<IStatementNode> parse_function();
	ParseResult<Block> parse_function_body();
};