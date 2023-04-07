#pragma once
#include "Sym.h"
#include "SymLookup.h"
#include "SymScope.h"

#include <deque>
#include <string>
class SymTab
{
	std::deque<Sym> syms;

	SymScope root;
	std::vector<SymScope*> stack;

public:
	SymTab();

	SymLookupResult lookup(NameParts name);

	void push_scope(SymScope* scope);
	void pop_scope();

	template<typename Node, typename... Args>
	Sym* create(Args&&... args);

	template<typename Node, typename... Args>
	Sym* create_named(std::string name, Args&&... args);

private:
	SymScope* current_scope();
};

template<typename Node, typename... Args>
Sym*
SymTab::create(Args&&... args)
{
	Sym* sym = &syms.emplace_back();
	sym->kind = Node::sk;

	if constexpr( std::is_same_v<SymVar, Node> )
		sym->data.sym_var = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<SymFunc, Node> )
		sym->data.sym_func = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<SymNamespace, Node> )
		sym->data.sym_namespace = Node(std::forward<Args>(args)...);
	else if constexpr( std::is_same_v<SymType, Node> )
		sym->data.sym_type = Node(std::forward<Args>(args)...);
	else
		static_assert("Cannot create symbol of type " + to_string(Node::sk));

	return sym;
}

template<typename Node, typename... Args>
Sym*
SymTab::create_named(std::string name, Args&&... args)
{
	Sym* sym = create<Node, Args...>(std::forward<Args>(args)...);

	SymScope* scope = current_scope();
	scope->insert(name, sym);

	return sym;
}