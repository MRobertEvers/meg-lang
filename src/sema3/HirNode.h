#pragma once
#include "QualifiedTy.h"
#include "Sym.h"
#include "Ty.h"
#include "ast3/NameParts.h"
#include "ast3/bin_op.h"

#include <vector>

enum class HirNodeKind
{
	Invalid,
	Module,
	Block,
	Call,
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
	std::vector<HirNode*> parameters;

	Sym* sym;

	// TODO: I'm not entirely sure we need to keep track of the rt_type_declarator
	// Maybe for some template things?
	HirFuncProto(Linkage linkage, Sym* sym, std::vector<HirNode*> parameters)
		: linkage(linkage)
		, parameters(parameters)
		, sym(sym)
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

	Sym* sym;

	HirId(Sym* sym)
		: sym(sym)
	{}
};

struct HirCall
{
	static constexpr HirNodeKind nt = HirNodeKind::Call;

	enum class CallKind
	{
		Invalid,
		Static,
		PtrCall,

		// For Builtin Functions that do special things
		// during codegen.
		BuiltIn
	} kind = CallKind::Invalid;

	// Only one is populated
	union
	{
		Sym* callee;
		HirNode* callee_expr;
		BinOp op;
	};

	std::vector<HirNode*> args;

	HirCall(Sym* callee, std::vector<HirNode*> args)
		: callee(callee)
		, args(args)
		, kind(CallKind::Static)
	{}

	HirCall(BinOp op, std::vector<HirNode*> args)
		: op(op)
		, args(args)
		, kind(CallKind::BuiltIn)
	{}

	HirCall(HirNode* callee_expr, std::vector<HirNode*> args)
		: callee_expr(callee_expr)
		, args(args)
		, kind(CallKind::PtrCall)
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

	long long value = 0;

	HirNumberLiteral(long long value)
		: value(value)
	{}
};

struct HirNode
{
	HirNodeKind kind = HirNodeKind::Invalid;

	QualifiedTy qty;

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
		HirCall hir_call;

		// Attention! This leaks!
		NodeData() {}
		// TODO: Deleter
		~NodeData() {}
	} data;
};
