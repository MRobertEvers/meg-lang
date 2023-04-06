#pragma once

#include "HirNode.h"

#include <deque>

class Hir
{
	std::deque<HirNode> nodes;

public:
	Hir();

	template<typename Node, typename... Args>
	HirNode* create(Args&&... args);
};

template<typename Node, typename... Args>
HirNode*
Hir::create(Args&&... args)
{
	HirNode* hir_node = &nodes.emplace_back();
	hir_node->kind = Node::nt;

	if constexpr( std::is_same_v<HirModule, Node> )
		hir_node->data.hir_module = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirReturn, Node> )
		hir_node->data.hir_return = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirNumberLiteral, Node> )
		hir_node->data.hir_number_literal = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirBlock, Node> )
		hir_node->data.hir_block = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirFunc, Node> )
		hir_node->data.hir_func = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirFuncProto, Node> )
		hir_node->data.hir_func_proto = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirId, Node> )
		hir_node->data.hir_id = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<HirTypeDeclarator, Node> )
		hir_node->data.hir_type_declarator = Node(std::forward<Args>(args)...);
	else
		static_assert("Cannot create hir node of type " + to_string(Node::nt));

	return hir_node;
}