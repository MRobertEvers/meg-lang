#pragma once
#include "QualifiedTy.h"
#include "Ty.h"
#include "ast3/NameParts.h"

#include <vector>

enum class HirNodeKind
{
	Invalid,
	Module,
	Block,
	Func,
	FuncProto,
	Id,
	Return,
	TypeDeclarator,
	VarDecl,
	Expr,
	Stmt,
	NumberLiteral
};

struct HirNode;

struct HirModule
{
	static constexpr HirNodeKind nt = HirNodeKind::Module;

	std::vector<HirNode*> statements;

	HirModule(std::vector<HirNode*> statements)
		: statements(statements)
	{}
};

struct HirBlock
{
	static constexpr HirNodeKind nt = HirNodeKind::Block;

	std::vector<HirNode*> statements;

	HirBlock(std::vector<HirNode*> statements)
		: statements(statements)
	{}
};

struct HirFunc
{
	static constexpr HirNodeKind nt = HirNodeKind::Func;

	HirNode* proto;
	HirNode* body;

	HirFunc(HirNode* proto, HirNode* body)
		: proto(proto)
		, body(body)
	{}
};

struct HirFuncProto
{
	static constexpr HirNodeKind nt = HirNodeKind::FuncProto;

	enum class Linkage
	{
		Extern,
		None
	};

	Linkage linkage = Linkage::None;
	HirNode* id;
	std::vector<HirNode*> parameters;
	HirNode* rt_type_declarator;

	Ty const* ty;

	// TODO: I'm not entirely sure we need to keep track of the rt_type_declarator
	// Maybe for some template things?
	HirFuncProto(
		Linkage linkage,
		HirNode* id,
		std::vector<HirNode*> parameters,
		HirNode* rt_type_declarator,
		Ty const* ty)
		: linkage(linkage)
		, parameters(parameters)
		, id(id)
		, rt_type_declarator(rt_type_declarator)
		, ty(ty)
	{}
};

struct HirReturn
{
	static constexpr HirNodeKind nt = HirNodeKind::Return;

	HirNode* expr;

	HirReturn(HirNode* expr)
		: expr(expr)
	{}
};

struct HirId
{
	static constexpr HirNodeKind nt = HirNodeKind::Id;

	NameParts name_parts;

	HirId(NameParts name_parts)
		: name_parts(name_parts)
	{}
};

struct HirTypeDeclarator
{
	static constexpr HirNodeKind nt = HirNodeKind::TypeDeclarator;

	QualifiedTy qty;

	HirTypeDeclarator(QualifiedTy qty)
		: qty(qty)
	{}
};

struct HirNumberLiteral
{
	static constexpr HirNodeKind nt = HirNodeKind::NumberLiteral;

	long long value;

	HirNumberLiteral(long long value)
		: value(value)
	{}
};

struct HirNode
{
	HirNodeKind kind = HirNodeKind::Invalid;
	union NodeData
	{
		HirModule hir_module;
		HirBlock hir_block;
		HirFunc hir_func;
		HirFuncProto hir_func_proto;
		HirReturn hir_return;
		HirId hir_id;
		HirTypeDeclarator hir_type_declarator;
		HirNumberLiteral hir_number_literal;

		// Attention! This leaks!
		NodeData() {}
		// TODO: Deleter
		~NodeData() {}
	} data;
};
