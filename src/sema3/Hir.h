#pragma once

#include "HirNode.h"
#include "QualifiedTy.h"

#include <deque>

class Hir
{
	std::deque<HirNode> nodes;

public:
	Hir();

	template<typename Node, typename... Args>
	HirNode* create(QualifiedTy qty, Args&&... args);
};

template<typename Node>
auto&
hir_cast(HirNode* hir_node)
{
	if constexpr( std::is_same_v<HirModule, Node> )
		return hir_node->data.hir_module;
	else if constexpr( std::is_same_v<HirReturn, Node> )
		return hir_node->data.hir_return;
	else if constexpr( std::is_same_v<HirNumberLiteral, Node> )
		return hir_node->data.hir_number_literal;
	else if constexpr( std::is_same_v<HirBlock, Node> )
		return hir_node->data.hir_block;
	else if constexpr( std::is_same_v<HirFunc, Node> )
		return hir_node->data.hir_func;
	else if constexpr( std::is_same_v<HirFuncProto, Node> )
		return hir_node->data.hir_func_proto;
	else if constexpr( std::is_same_v<HirId, Node> )
		return hir_node->data.hir_id;
	else if constexpr( std::is_same_v<HirCall, Node> )
		return hir_node->data.hir_call;
	else if constexpr( std::is_same_v<HirLet, Node> )
		return hir_node->data.hir_let;
	else if constexpr( std::is_same_v<HirIf, Node> )
		return hir_node->data.hir_if;
	else if constexpr( std::is_same_v<HirStruct, Node> )
		return hir_node->data.hir_struct;
	else if constexpr( std::is_same_v<HirUnion, Node> )
		return hir_node->data.hir_union;
	else if constexpr( std::is_same_v<HirEnum, Node> )
		return hir_node->data.hir_enum;
	else if constexpr( std::is_same_v<HirSubscript, Node> )
		return hir_node->data.hir_subscript;
	else
		static_assert("Cannot create hir node of type ");
}

template<typename Node, typename... Args>
HirNode*
Hir::create(QualifiedTy qty, Args&&... args)
{
	HirNode* hir_node = &nodes.emplace_back();
	hir_node->kind = Node::nt;
	hir_node->qty = qty;

	hir_cast<Node>(hir_node) = Node(std::forward<Args>(args)...);

	return hir_node;
}
