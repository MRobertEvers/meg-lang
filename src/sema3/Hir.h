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
