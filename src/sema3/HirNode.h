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
	Subscript, // arr[]
	Member,	   // a.b or a->b
	Return,
	Struct,
	Union,
	Enum,
	Let,
	If,
	Loop,
	Switch,
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
		// For scoped blocks, defer and destructors are inserted
		// for variables constructed within the scope.
		Scoped,

		// Unscoped blocks do not do this.
		Inherit
	} scoping = Scoping::Scoped;

	// The last statement in a block is returned as the result of
	// the block so long as the statement is also an expression.
	// I.e. a non-void Node.
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
		AddressOf,
		BoolNot,
		Deref,

		// Args { HirId, HirNumberLiteral }
		// Checks if the type (must be ENUM) tag for the HirId arg
		// is the same as HirNumberLiteral
		Is
	};

	enum class CallKind
	{
		Invalid,
		// When a function is called by name.
		// e.g. fn my_func()... let x = my_func()
		Static,

		// When a function is called with ptr indirection
		// e.g. a.b() or let x = &my_func; x();
		PtrCall,

		// Similar to Builtin, but separated because
		// BinOps are all fairly similar. BuiltIn's can
		// do weird things.
		BinOp,

		// Catchall for Builtin Functions that do special things
		// during codegen.
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
	static constexpr HirNodeKind nt = HirNodeKind::Struct;

	Sym* sym;

	HirStruct(Sym* sym)
		: sym(sym)
	{}
};

struct HirUnion
{
	static constexpr HirNodeKind nt = HirNodeKind::Union;

	Sym* sym;

	HirUnion(Sym* sym)
		: sym(sym)
	{}
};

struct HirEnum
{
	static constexpr HirNodeKind nt = HirNodeKind::Enum;

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

/**
 * Used to perform pointer arithmetic.
 * Binary operations on pointers are transformed into HirSubscript
 *
 */
struct HirSubscript
{
	static constexpr HirNodeKind nt = HirNodeKind::Subscript;

	HirNode* elem;
	HirNode* subscript;

	HirSubscript(HirNode* elem, HirNode* subscript)
		: elem(elem)
		, subscript(subscript)
	{}
};

/**
 * Unlike AstIf, HirIf can be used as an expression.
 * HirIf branches must all have the same return type in order to
 * be properly formed.
 *
 * Additionally, if there is no else block,
 * the expression itself is returned as the expression.
 *
 * AstIfs are converted in to void HirIfs, boolean expressions
 * are converted into bool HirIfs
 *
 */
struct HirIf
{
	static constexpr HirNodeKind nt = HirNodeKind::If;

	struct CondThen
	{
		HirNode* cond;

		// For non-void ifs, the result of this node
		// is returned.
		HirNode* then;
	};

	std::vector<CondThen> elsifs;

	HirNode* else_node = nullptr;
	// If cond_else is true, include the expr in the phi resolution.
	// Note! Incompatible with else block.
	bool cond_else = false;

	HirIf(std::vector<CondThen> elsifs, HirNode* else_node)
		: elsifs(elsifs)
		, else_node(else_node)
	{}
	HirIf(std::vector<CondThen> elsifs, bool cond_else)
		: elsifs(elsifs)
		, cond_else(cond_else)
	{}
};

struct HirSwitch
{
	static constexpr HirNodeKind nt = HirNodeKind::Switch;

	HirNode* cond;

	struct CondThen
	{
		long long value;
		HirNode* then;
	};

	std::vector<CondThen> branches;
	HirNode* default_branch;

	HirSwitch(HirNode* cond, std::vector<CondThen> branches, HirNode* default_branch)
		: cond(cond)
		, branches(branches)
		, default_branch(default_branch)
	{}
};

struct HirMember
{
	static constexpr HirNodeKind nt = HirNodeKind::Member;

	enum AccessKind
	{
		Direct,
		Indirect
	} kind;

	HirNode* self;
	Sym* member;

	HirMember(HirNode* self, Sym* member, AccessKind kind)
		: self(self)
		, member(member)
		, kind(kind)
	{}
};

struct HirLoop
{
	static constexpr HirNodeKind nt = HirNodeKind::Loop;

	HirNode* init; // For do-while, the first iteration is put in here.
	HirNode* cond;
	HirNode* body;
	HirNode* after;

	HirLoop(HirNode* init, HirNode* cond, HirNode* body, HirNode* after)
		: init(init)
		, cond(cond)
		, body(body)
		, after(after)
	{}
};

struct Inferrence
{
	Sym* sym;
	QualifiedTy qty;
};

struct HirNode
{
	HirNodeKind kind = HirNodeKind::Invalid;

	QualifiedTy qty;

	/**
	 * This is used to track type inferring statements.
	 * In if clauses, if a type becomes inferred.
	 */
	std::vector<Inferrence> inferrences;

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
		HirSubscript hir_subscript;
		HirMember hir_member;
		HirSwitch hir_switch;
		HirLoop hir_loop;

		// Attention! This leaks!
		NodeData() {}
		// TODO: Deleter
		~NodeData() {}
	} data;
};
