#pragma once
#include "QualifiedTy.h"
#include "SymScope.h"
#include "Ty.h"

enum class SymKind
{
	Invalid,
	Var,
	Alias,
	Template,
	TemplateParameter,
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

struct SymMember
{
	static constexpr SymKind sk = SymKind::Member;
	QualifiedTy qty;

	// For structs this is used to order members in memory.
	int position = 0;

	SymMember(QualifiedTy qty, int position);
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

	SymTemplate();
};

struct SymTemplateParameter
{
	static constexpr SymKind sk = SymKind::TemplateParameter;

	SymTemplateParameter();
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
		SymTemplate sym_template;
		SymTemplateParameter sym_template_parameter;

		// Attention! This leaks!
		SymData() {}
		// TODO: Deleter
		~SymData() {}
	} data; //
};

Sym* sym_unalias(Sym* sym);