#pragma once

#include "Ast.h"
#include "AstNode.h"
#include "ParseResult.h"
#include "ParseTrail.h"
#include "lexer/TokenCursor.h"

namespace ast
{
class AstGen
{
public:
	Ast& ast;
	TokenCursor& cursor;
	ParserMetaInformation meta;

	AstGen(Ast& ast, TokenCursor& cursor);

	ParseResult<AstNode*> parse();

	ParseResult<AstNode*> parse_module_top_level_item();

	ParseResult<AstNode*> parse_let();
	ParseResult<AstNode*> parse_block();
	ParseResult<AstNode*> parse_union();
	ParseResult<AstNode*> parse_type_decl(bool allow_empty);

	ParseResult<AstNode*> parse_bin_op(int ExprPrec, AstNode* lhs);
	ParseResult<AstNode*> parse_assign(AstNode* lhs);
	ParseResult<AstNode*> parse_expr_statement();
	ParseResult<AstNode*> parse_while();
	ParseResult<AstNode*> parse_switch();
	ParseResult<AstNode*> parse_yield();
	ParseResult<AstNode*> parse_case();
	ParseResult<AstNode*> parse_default();
	ParseResult<AstNode*> parse_statement();

	ParseResult<AstNode*> parse_if();
	ParseResult<AstNode*> parse_else();
	ParseResult<AstNode*> parse_for();
	ParseResult<AstNode*> parse_literal();
	// ParseResult<AstNode*> parse_type_identifier();
	ParseResult<AstNode*> parse_identifier();
	ParseResult<AstNode*> parse_const_expr();
	ParseResult<AstNode*> parse_initializer(AstNode* identifier);
	ParseResult<AstNode*> parse_value_list();
	ParseResult<AstNode*> parse_call(AstNode* base);
	ParseResult<AstNode*> parse_indirect_member_reference(AstNode* base);
	ParseResult<AstNode*> parse_member_reference(AstNode* base);
	ParseResult<AstNode*> parse_array_access(AstNode* base);

	ParseResult<AstNode*> parse_deref();
	ParseResult<AstNode*> parse_boolnot();
	ParseResult<AstNode*> parse_addressof();
	ParseResult<AstNode*> parse_simple_expr();
	ParseResult<AstNode*> parse_postfix_expr();
	ParseResult<AstNode*> parse_expr();

	ParseResult<AstNode*> parse_non_var_arg_fn_param();
	ParseResult<AstNode*> parse_fn_param();
	ParseResult<AstNode*> parse_function_parameter_list();
	ParseResult<AstNode*> parse_function_proto();
	ParseResult<AstNode*> parse_extern_function();
	ParseResult<AstNode*> parse_function();
	ParseResult<AstNode*> parse_function_body();

	ParseTrail get_parse_trail();
};
} // namespace ast