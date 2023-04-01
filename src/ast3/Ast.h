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

	AstNode* root();

	template<typename Node, typename... Args>
	AstNode* create(Span span, Args&&... args);

	// TODO: Destructor for a node.
	// static void reset_node();
};

template<typename Node, typename... Args>
AstNode*
Ast::create(Span span, Args&&... args)
{
	AstNode* ast_node = &nodes.emplace_back();
	ast_node->span = span;
	ast_node->type = Node::nt;

	if constexpr( std::is_same_v<AstModule, Node> )
		ast_node->data.ast_module = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstId, Node> )
		ast_node->data.ast_id = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstVarDecl, Node> )
		ast_node->data.ast_var_decl = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstBlock, Node> )
		ast_node->data.ast_block = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstFunc, Node> )
		ast_node->data.ast_func = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstFuncProto, Node> )
		ast_node->data.ast_func_proto = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstTypeDeclarator, Node> )
		ast_node->data.ast_type_declarator = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<AstReturn, Node> )
		ast_node->data.ast_return = Node(std::forward<Args>(args)...);
	else
		static_assert("Cannot create node of type " + to_string(Node::nt));

	return ast_node;
}