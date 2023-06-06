#include "Sym.h"

Sym*
sym_unalias(Sym* sym)
{
	while( sym->kind == SymKind::Alias )
		sym = sym->data.sym_alias.sym;

	return sym;
}

SymTemplate::SymTemplate(SymKind kind, std::vector<AstNode*> typenames, AstNode* template_tree)
	: template_kind(kind)
	, typenames(typenames)
	, template_tree(template_tree){};

SymVar::SymVar(QualifiedTy qty)
	: qty(qty){};

SymAlias::SymAlias(Sym* sym)
	: sym(sym){};

// SymTypeAlias::SymTypeAlias(QualifiedTy qty)
// 	: qty(qty){};

SymFunc::SymFunc(Ty const* ty)
	: ty(ty){};

SymType::SymType(Ty const* ty)
	: ty(ty){};

SymMember::SymMember(Member mem)
	: member(mem){};

SymEnumMember::SymEnumMember(long long value, Ty const* struct_ty)
	: value(value)
	, struct_ty(struct_ty)
	, kind(MemberKind::Struct){};

SymEnumMember::SymEnumMember(long long value)
	: value(value)
	, struct_ty(nullptr)
	, kind(MemberKind::Simple){};

SymNamespace::SymNamespace(){};
