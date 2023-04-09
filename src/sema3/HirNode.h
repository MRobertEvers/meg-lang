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
	Struct,
	Union,
	Enum,
	Let,
	If,
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

	enum class Scoping
	{
		Scoped,
		Inherit
	} scoping = Scoping::Scoped;

	std::vector<HirNode*> statements;

	HirBlock(std::vector<HirNode*> statements, Scoping scoping)
		: statements(statements)
		, scoping(scoping)
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

	enum class BuiltinKind
	{
		Invalid,
		Cast,
		SizeOf,
	};

	enum class CallKind
	{
		Invalid,
		Static,
		PtrCall,

		// For Builtin Functions that do special things
		// during codegen.
		BinOp,
		BuiltIn
	} kind = CallKind::Invalid;

	// Only one is populated
	union
	{
		Sym* callee;
		HirNode* callee_expr;
		BinOp op;
		BuiltinKind builtin;
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
		, kind(CallKind::BinOp)
	{}

	HirCall(BuiltinKind builtin, std::vector<HirNode*> args)
		: builtin(builtin)
		, args(args)
		, kind(CallKind::BuiltIn)
	{}

	HirCall(HirNode* callee_expr, std::vector<HirNode*> args)
		: callee_expr(callee_expr)
		, args(args)
		, kind(CallKind::PtrCall)
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

struct HirStruct
{
	static constexpr HirNodeKind nt = HirNodeKind::Let;

	Sym* sym;

	HirStruct(Sym* sym)
		: sym(sym)
	{}
};

struct HirUnion
{
	static constexpr HirNodeKind nt = HirNodeKind::Let;

	Sym* sym;

	HirUnion(Sym* sym)
		: sym(sym)
	{}
};

struct HirEnum
{
	static constexpr HirNodeKind nt = HirNodeKind::Let;

	Sym* sym;

	HirEnum(Sym* sym)
		: sym(sym)
	{}
};

struct HirLet
{
	static constexpr HirNodeKind nt = HirNodeKind::Let;

	Sym* sym;

	HirLet(Sym* sym)
		: sym(sym)
	{}
};

struct HirIf
{
	static constexpr HirNodeKind nt = HirNodeKind::If;

	struct CondThen
	{
		HirNode* cond;
		HirNode* then;
	};

	std::vector<CondThen> elsifs;
	HirNode* else_node;

	HirIf(std::vector<CondThen> elsifs, HirNode* else_node)
		: elsifs(elsifs)
		, else_node(else_node)
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
		HirNumberLiteral hir_number_literal;
		HirCall hir_call;
		HirLet hir_let;
		HirIf hir_if;
		HirStruct hir_struct;
		HirUnion hir_union;
		HirEnum hir_enum;

		// Attention! This leaks!
		NodeData() {}
		// TODO: Deleter
		~NodeData() {}
	} data;
};
