#pragma once
#include "../../ast/ast.h"
#include "../../lexer/TokenCursor.h"
#include "../ParseResult.h"
#include "ParseTrail.h"
#include "common/OwnPtr.h"

using namespace ast;

namespace parser
{
class Parser
{
	TokenCursor& cursor;

	ParserMetaInformation meta;

public:
	Parser(TokenCursor& cursor);

	ParseResult<ast::Module> parse_module();

private:
	ParseResult<IStatementNode> parse_module_top_level_item();

	ParseResult<Let> parse_let();
	ParseResult<Block> parse_block();
	ParseResult<Struct> parse_struct();
	ParseResult<TypeDeclarator> parse_type_decl(bool allow_empty);

	ParseResult<IExpressionNode> parse_bin_op(int ExprPrec, OwnPtr<IExpressionNode> LHS);
	ParseResult<Assign> parse_assign(OwnPtr<IExpressionNode> lhs);
	ParseResult<IStatementNode> parse_expr_statement();
	ParseResult<While> parse_while();
	ParseResult<IStatementNode> parse_statement();

	ParseResult<If> parse_if();
	ParseResult<For> parse_for();
	ParseResult<IExpressionNode> parse_literal();
	ParseResult<TypeIdentifier> parse_type_identifier();
	ParseResult<ValueIdentifier> parse_identifier();
	ParseResult<ArgumentList> parse_value_list();
	ParseResult<Call> parse_call(OwnPtr<IExpressionNode> base);
	ParseResult<MemberReference> parse_member_reference(OwnPtr<IExpressionNode> base);

	ParseResult<IExpressionNode> parse_simple_expr();
	ParseResult<IExpressionNode> parse_postfix_expr();
	ParseResult<IExpressionNode> parse_expr();

	ParseResult<ParameterList> parse_function_parameter_list();
	ParseResult<Prototype> parse_function_proto();
	ParseResult<Function> parse_function();
	ParseResult<Block> parse_function_body();

	ParseTrail get_parse_trail();
};
} // namespace parser