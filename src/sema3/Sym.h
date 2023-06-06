#pragma once
#include "QualifiedTy.h"
#include "SymScope.h"
#include "Ty.h"
#include "ast3/Ast.h"

#include <string>

enum class SymKind
{
	Invalid,
	Var,
	Alias,
	// TypeAlias, // For aliasing a type like Point*.
	Template,
	Member,
	EnumMember,
	Type,
	Func,
	Namespace
};

struct SymVar
{
	static constexpr SymKind sk = SymKind::Var;
	QualifiedTy qty;

	SymVar(QualifiedTy qty);
};

struct SymAlias
{
	static constexpr SymKind sk = SymKind::Alias;
	Sym* sym;

	SymAlias(Sym* sym);
};

// struct SymTypeAlias
// {
// 	static constexpr SymKind sk = SymKind::TypeAlias;
// 	QualifiedTy qty;

// 	SymTypeAlias(QualifiedTy qty);
// };

struct SymMember
{
	static constexpr SymKind sk = SymKind::Member;

	Member member;

	SymMember(Member member);
};

struct SymEnumMember
{
	static constexpr SymKind sk = SymKind::EnumMember;

	enum class MemberKind
	{
		Simple,
		Struct
	} kind = MemberKind::Simple;

	union
	{
		Ty const* struct_ty;
	};

	long long value;

	SymEnumMember(long long value, Ty const* struct_ty);
	SymEnumMember(long long value);
};

struct SymFunc
{
	static constexpr SymKind sk = SymKind::Func;
	// Right now this contains parameters.
	// Eventually, this wont be here as we
	// will have to perform overload resolution first.
	SymScope scope;
	Ty const* ty;

	SymFunc(Ty const* ty);
};

struct SymType
{
	static constexpr SymKind sk = SymKind::Type;
	SymScope scope;
	Ty const* ty;

	SymType(Ty const* ty);
};

struct SymTemplate
{
	static constexpr SymKind sk = SymKind::Template;

	struct Instance
	{
		std::vector<QualifiedTy> types;
		Sym* sym;
	};
	std::vector<Instance> instances;

	SymKind template_kind = SymKind::Invalid;

	std::vector<AstNode*> typenames;
	AstNode* template_tree;

	SymTemplate(SymKind kind, std::vector<AstNode*> typenames, AstNode* template_tree);
};

struct SymNamespace
{
	static constexpr SymKind sk = SymKind::Namespace;
	SymScope scope;

	SymNamespace();
};

struct Sym
{
	SymKind kind = SymKind::Invalid;
	union SymData
	{
		SymVar sym_var;
		SymFunc sym_func;
		SymNamespace sym_namespace;
		SymType sym_type;
		SymMember sym_member;
		SymEnumMember sym_enum_member;
		SymAlias sym_alias;
		// SymTypeAlias sym_type_alias;
		SymTemplate sym_template;

		// Attention! This leaks!
		SymData() {}
		// TODO: Deleter
		~SymData() {}
	} data; //
};

Sym* sym_unalias(Sym* sym);

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
	else if constexpr( std::is_same_v<SymAlias, Node> )
		return sym->data.sym_alias;
	// else if constexpr( std::is_same_v<SymTypeAlias, Node> )
	// 	return sym->data.sym_type_alias;
	else if constexpr( std::is_same_v<SymTemplate, Node> )
		return sym->data.sym_template;
	else
		static_assert("Cannot create symbol of type " + to_string(Node::sk));
}
