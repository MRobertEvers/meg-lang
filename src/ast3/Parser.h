#pragma once

#include "ParseResult.h"
#include "ast3/Ast.h"
#include "ast3/AstNode.h"
#include "lex3/Cursor.h"

#include <string>
#include <vector>

class Parser
{
	Ast& ast;
	Cursor& cursor;

public:
	Parser(Ast& ast, Cursor& cursor)
		: ast(ast)
		, cursor(cursor)
	{}

	ParseResult<AstNode*> parse_module();
	ParseResult<AstNode*> parse_module_statement();
	ParseResult<AstNode*> parse_identifier();
	ParseResult<AstNode*> parse_type_decl(bool allow_empty);
	ParseResult<AstNode*> parse_var_decl(bool allow_untyped);
	ParseResult<AstNode*> parse_call(AstNode* callee);
	ParseResult<AstNode*> parse_member_access(AstNode* base);
	ParseResult<AstNode*> parse_array_access(AstNode* array);
	ParseResult<AstNode*> parse_deref();
	ParseResult<AstNode*> parse_function();
	ParseResult<AstNode*> parse_function_proto();
	ParseResult<AstNode*> parse_function_body();
	ParseResult<AstNode*> parse_block();
	ParseResult<AstNode*> parse_struct();
	ParseResult<AstNode*> parse_union();
	ParseResult<AstNode*> parse_enum();
	ParseResult<AstNode*> parse_enum_member();
	ParseResult<AstNode*> parse_let();
	ParseResult<AstNode*> parse_if();
	ParseResult<AstNode*> parse_number_literal();
	ParseResult<AstNode*> parse_postfix_expr();
	ParseResult<AstNode*> parse_simple_expr();
	ParseResult<AstNode*> parse_bin_op(int precidence, AstNode* lhs);
	ParseResult<AstNode*> parse_expr();
	ParseResult<AstNode*> parse_expr_any();
	ParseResult<AstNode*> parse_statement();

	static ParseResult<AstNode*> parse(Ast& ast, Cursor& cursor);

private:
	ParseResult<std::vector<AstNode*>> parse_struct_body();
	ParseResult<std::vector<std::string>> parse_name_parts();
	ParseResult<std::vector<AstNode*>> parse_func_params();
};
