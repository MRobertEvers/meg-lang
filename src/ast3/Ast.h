#pragma once
#include "AstNode.h"
#include "Span.h"

#include <deque>

class Ast
{
private:
	std::deque<AstNode> nodes;

public:
	Ast();

	template<typename Node, typename... Args>
	AstNode* create(Span span, Args&&... args);

	// TODO: Destructor for a node.
	// static void reset_node();
};

template<typename Node, typename AstTy>
auto&
ast_cast(AstTy* ast_node)
{
	assert(ast_node->kind == Node::nt && "Bad ast_cast");

	if constexpr( std::is_same_v<AstModule, Node> )
		return ast_node->data.ast_module;
	else if constexpr( std::is_same_v<AstId, Node> )
		return ast_node->data.ast_id;
	else if constexpr( std::is_same_v<AstVarDecl, Node> )
		return ast_node->data.ast_var_decl;
	else if constexpr( std::is_same_v<AstBlock, Node> )
		return ast_node->data.ast_block;
	else if constexpr( std::is_same_v<AstFunc, Node> )
		return ast_node->data.ast_func;
	else if constexpr( std::is_same_v<AstFuncProto, Node> )
		return ast_node->data.ast_func_proto;
	else if constexpr( std::is_same_v<AstTypeDeclarator, Node> )
		return ast_node->data.ast_type_declarator;
	else if constexpr( std::is_same_v<AstReturn, Node> )
		return ast_node->data.ast_return;
	else if constexpr( std::is_same_v<AstExpr, Node> )
		return ast_node->data.ast_expr;
	else if constexpr( std::is_same_v<AstStmt, Node> )
		return ast_node->data.ast_stmt;
	else if constexpr( std::is_same_v<AstNumberLiteral, Node> )
		return ast_node->data.ast_number_literal;
	else if constexpr( std::is_same_v<AstFuncCall, Node> )
		return ast_node->data.ast_func_call;
	else if constexpr( std::is_same_v<AstBinOp, Node> )
		return ast_node->data.ast_bin_op;
	else if constexpr( std::is_same_v<AstLet, Node> )
		return ast_node->data.ast_let;
	else if constexpr( std::is_same_v<AstIf, Node> )
		return ast_node->data.ast_if;
	else if constexpr( std::is_same_v<AstStruct, Node> )
		return ast_node->data.ast_struct;
	else if constexpr( std::is_same_v<AstUnion, Node> )
		return ast_node->data.ast_union;
	else if constexpr( std::is_same_v<AstEnum, Node> )
		return ast_node->data.ast_enum;
	else if constexpr( std::is_same_v<AstSizeOf, Node> )
		return ast_node->data.ast_sizeof;
	else if constexpr( std::is_same_v<AstEnumMember, Node> )
		return ast_node->data.ast_enum_member;
	else
		static_assert("Cannot create node of type " + to_string(Node::nt));
}

template<typename Node, typename... Args>
AstNode*
Ast::create(Span span, Args&&... args)
{
	AstNode* ast_node = &nodes.emplace_back();
	ast_node->span = span;
	ast_node->kind = Node::nt;

	ast_cast<Node>(ast_node) = Node(std::forward<Args>(args)...);

	return ast_node;
}
