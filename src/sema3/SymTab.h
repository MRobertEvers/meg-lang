#pragma once
#include "Sym.h"
#include "SymLookup.h"
#include "SymScope.h"

#include <deque>
#include <map>
#include <string>

class SymTab
{
	std::deque<Sym> syms;
	std::deque<SymScope> free_scopes;

	SymScope root;
	std::vector<SymScope*> stack;
	std::map<Ty const*, Sym*> ty_lookup;

public:
	SymTab();

	SymLookupResult lookup(NameParts name);
	SymLookupResult lookup(Ty const* ty);

	void push_scope(SymScope* scope);
	void push_scope();
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

	sym_cast<Node>(sym) = Node(std::forward<Args>(args)...);

	if constexpr( std::is_same_v<SymType, Node> )
		ty_lookup.emplace(sym_cast<SymType>(sym).ty, sym);
	else if constexpr( std::is_same_v<SymFunc, Node> )
		ty_lookup.emplace(sym_cast<SymFunc>(sym).ty, sym);
	else if constexpr( std::is_same_v<SymEnumMember, Node> )
	{
		SymEnumMember& member = sym_cast<SymEnumMember>(sym);
		ty_lookup.emplace(member.struct_ty, sym);
	}

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
