#pragma once
#include "../../ast/ast.h"
#include "../../lexer/TokenCursor.h"
#include "../ParseResult.h"
#include "common/OwnPtr.h"

using namespace ast;

// class ParseScope
// {
// 	ParseScope* parent = nullptr;

// 	std::map<String, Type const*> names;

// public:
// 	ParseScope(){};
// 	ParseScope(ParseScope* parent)
// 		: parent(parent){};

// 	void add_name(String const& name, Type const* type);
// 	Type const* get_type_for_name(String const& name);

// 	ParseScope* get_parent() { return parent; }

// 	static ParseScope* CreateDefault();
// };

class Parser
{
	TokenCursor& cursor;

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
	ParseResult<IExpressionNode> parse_literal();
	ParseResult<ValueIdentifier> parse_identifier();
	ParseResult<Vec<OwnPtr<IExpressionNode>>> parse_value_list();
	ParseResult<Call> parse_call(OwnPtr<IExpressionNode> base);
	ParseResult<MemberReference> parse_member_reference(OwnPtr<IExpressionNode> base);
	// ParseResult<MemberReference> parse_function_call();
	ParseResult<IExpressionNode> parse_simple_expr();
	ParseResult<IExpressionNode> parse_postfix_expr();
	ParseResult<IExpressionNode> parse_expr();

	ParseResult<Vec<OwnPtr<ParameterDeclaration>>> parse_function_parameter_list();
	ParseResult<Prototype> parse_function_proto();
	ParseResult<Function> parse_function();
	ParseResult<Block> parse_function_body();

	// void pop_scope();
	// void new_scope();
};