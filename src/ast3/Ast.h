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
