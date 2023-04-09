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

	SymScope root;
	std::vector<SymScope*> stack;
	std::map<Ty const*, Sym*> ty_lookup;

public:
	SymTab();

	SymLookupResult lookup(NameParts name);
	SymLookupResult lookup(Ty const* ty);

	void push_scope(SymScope* scope);
	void pop_scope();

	template<typename Node, typename... Args>
	Sym* create(Args&&... args);

	template<typename Node, typename... Args>
	Sym* create_named(std::string name, Args&&... args);

private:
	SymScope* current_scope();
};

template<typename Node, typename SymTy>
auto&
sym_cast(SymTy* sym)
{
	assert(sym->kind == Node::sk);

	if constexpr( std::is_same_v<SymVar, Node> )
		return sym->data.sym_var;
	else if constexpr( std::is_same_v<SymFunc, Node> )
		return sym->data.sym_func;
	else if constexpr( std::is_same_v<SymNamespace, Node> )
		return sym->data.sym_namespace;
	else if constexpr( std::is_same_v<SymType, Node> )
		return sym->data.sym_type;
	else if constexpr( std::is_same_v<SymMember, Node> )
		return sym->data.sym_member;
	else if constexpr( std::is_same_v<SymEnumMember, Node> )
		return sym->data.sym_enum_member;
	else
		static_assert("Cannot create symbol of type " + to_string(Node::sk));
}

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