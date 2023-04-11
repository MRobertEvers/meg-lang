#pragma once
#include "NameParts.h"
#include "Span.h"
#include "bin_op.h"

#include <string>
#include <vector>

enum class NodeKind
{
	Invalid,
	Module,
	Func,
	FuncProto,
	FuncCall,
	ArrayAccess,
	MemberAccess,
	Deref,
	Block,
	Id,
	SizeOf,
	AddressOf,
	BoolNot,
	BinOp,
	Struct,
	Union,
	Enum,
	EnumMember,
	NumberLiteral,
	BoolLiteral,
	TypeDeclarator,
	Let,
	If,
	Return,
	VarDecl,
	Assign,
	Switch,
	Case,
	Break,
	Default,
	Continue,
	For,
	While,
	Expr,
	Stmt,
};

std::string to_string(NodeKind type);

struct AstNode;

struct AstModule
{
	static constexpr NodeKind nt = NodeKind::Module;

	std::vector<AstNode*> statements;

	AstModule(std::vector<AstNode*> statements)
		: statements(statements)
	{}
};

struct AstBlock
{
	static constexpr NodeKind nt = NodeKind::Block;

	std::vector<AstNode*> statements;

	AstBlock(std::vector<AstNode*> statements)
		: statements(statements)
	{}
};

struct AstTypeDeclarator
{
	static constexpr NodeKind nt = NodeKind::TypeDeclarator;
	AstNode* id;

	AstTypeDeclarator(AstNode* id)
		: id(id)
	{}
};

// simple_id is just a string
// [<simple_id> "::"]+ <simple_id>
struct AstId
{
	static constexpr NodeKind nt = NodeKind::Id;
	NameParts name_parts;

	AstId(std::string name)
		: name_parts({name})
	{}

	AstId(std::vector<std::string> name_parts)
		: name_parts(name_parts)
	{}
};

struct AstSizeOf
{
	static constexpr NodeKind nt = NodeKind::SizeOf;
	AstNode* expr;

	AstSizeOf(AstNode* expr)
		: expr(expr)
	{}
};

struct AstAddressOf
{
	static constexpr NodeKind nt = NodeKind::AddressOf;
	AstNode* expr;

	AstAddressOf(AstNode* expr)
		: expr(expr)
	{}
};

struct AstBoolNot
{
	static constexpr NodeKind nt = NodeKind::BoolNot;
	AstNode* expr;

	AstBoolNot(AstNode* expr)
		: expr(expr)
	{}
};

struct AstNumberLiteral
{
	static constexpr NodeKind nt = NodeKind::NumberLiteral;

	long long value;

	AstNumberLiteral(long long value)
		: value(value)
	{}
};

struct AstBoolLiteral
{
	static constexpr NodeKind nt = NodeKind::BoolLiteral;

	bool value;

	AstBoolLiteral(bool value)
		: value(value)
	{}
};

// <ast_id> [":" <ast_type_declarator>]
struct AstVarDecl
{
	static constexpr NodeKind nt = NodeKind::VarDecl;
	AstNode* id;
	AstNode* type_declarator;

	AstVarDecl(AstNode* id, AstNode* type_declarator)
		: id(id)
		, type_declarator(type_declarator)
	{}
};

// ["extern "] "fn " <ast_id>
// "(" [<ast_var_decl> ["," <ast_var_decl>]+] ")"
// [":" <ast_type_declarator>] ";"
struct AstFuncProto
{
	enum class Linkage
	{
		Extern,
		None
	};

	static constexpr NodeKind nt = NodeKind::FuncProto;

	Linkage linkage = Linkage::None;

	AstNode* id;
	std::vector<AstNode*> parameters;
	AstNode* rt_type_declarator;

	AstFuncProto(
		Linkage linkage, AstNode* id, std::vector<AstNode*> parameters, AstNode* rt_type_declarator)
		: linkage(linkage)
		, parameters(parameters)
		, id(id)
		, rt_type_declarator(rt_type_declarator)
	{}
};
struct AstFuncCall
{
	static constexpr NodeKind nt = NodeKind::FuncCall;

	AstNode* callee;
	std::vector<AstNode*> args;

	AstFuncCall(AstNode* callee, std::vector<AstNode*> args)
		: args(args)
		, callee(callee)
	{}
};

struct AstArrayAccess
{
	static constexpr NodeKind nt = NodeKind::ArrayAccess;

	AstNode* callee;
	AstNode* expr;

	AstArrayAccess(AstNode* callee, AstNode* expr)
		: expr(expr)
		, callee(callee)
	{}
};

struct AstDeref
{
	static constexpr NodeKind nt = NodeKind::Deref;

	AstNode* expr;

	AstDeref(AstNode* expr)
		: expr(expr)
	{}
};

struct AstMemberAccess
{
	static constexpr NodeKind nt = NodeKind::MemberAccess;

	enum class AccessKind
	{
		Direct,
		Indirect
	} kind = AccessKind::Direct;

	AstNode* callee;
	AstNode* id;

	AstMemberAccess(AstNode* callee, AstNode* id, AccessKind kind)
		: id(id)
		, callee(callee)
		, kind(kind)
	{}
};

struct AstFunc
{
	static constexpr NodeKind nt = NodeKind::Func;

	AstNode* proto;
	AstNode* body;

	AstFunc(AstNode* proto, AstNode* body)
		: proto(proto)
		, body(body)
	{}
};

struct AstReturn
{
	static constexpr NodeKind nt = NodeKind::Return;

	AstNode* expr;

	AstReturn(AstNode* expr)
		: expr(expr)
	{}
};

struct AstLet
{
	static constexpr NodeKind nt = NodeKind::Let;

	AstNode* var_decl;
	AstNode* rhs;

	AstLet(AstNode* var_decl, AstNode* rhs)
		: var_decl(var_decl)
		, rhs(rhs)
	{}
};

struct AstIf
{
	static constexpr NodeKind nt = NodeKind::If;

	AstNode* condition;
	AstNode* then_stmt;
	AstNode* else_stmt;

	AstIf(AstNode* condition, AstNode* then_stmt, AstNode* else_stmt)
		: condition(condition)
		, then_stmt(then_stmt)
		, else_stmt(else_stmt)
	{}
};

struct AstExpr
{
	static constexpr NodeKind nt = NodeKind::Expr;

	AstNode* expr;

	AstExpr() = default;
	AstExpr(AstNode* expr)
		: expr(expr)
	{}
};

struct AstStmt
{
	static constexpr NodeKind nt = NodeKind::Stmt;

	AstNode* stmt;

	AstStmt() = default;
	AstStmt(AstNode* stmt)
		: stmt(stmt)
	{}
};

struct AstStruct
{
	static constexpr NodeKind nt = NodeKind::Struct;

	AstNode* id;
	std::vector<AstNode*> var_decls;

	AstStruct() = default;
	AstStruct(AstNode* id, std::vector<AstNode*> var_decls)
		: id(id)
		, var_decls(var_decls)
	{}
};

struct AstUnion
{
	static constexpr NodeKind nt = NodeKind::Union;

	AstNode* id;
	std::vector<AstNode*> var_decls;

	AstUnion() = default;
	AstUnion(AstNode* id, std::vector<AstNode*> var_decls)
		: id(id)
		, var_decls(var_decls)
	{}
};

struct AstEnum
{
	static constexpr NodeKind nt = NodeKind::Enum;

	AstNode* id;
	std::vector<AstNode*> enum_member_decls;

	AstEnum() = default;
	AstEnum(AstNode* id, std::vector<AstNode*> enum_member_decls)
		: id(id)
		, enum_member_decls(enum_member_decls)
	{}
};

struct AstEnumMember
{
	static constexpr NodeKind nt = NodeKind::EnumMember;

	enum class MemberKind
	{
		Simple,
		Struct,
	} kind = MemberKind::Simple;

	AstNode* id;

	// Optional
	std::vector<AstNode*> var_decls;

	AstEnumMember() = default;
	AstEnumMember(AstNode* id)
		: id(id)
		, kind(MemberKind::Simple)
	{}
	AstEnumMember(AstNode* id, std::vector<AstNode*> var_decls)
		: id(id)
		, var_decls(var_decls)
		, kind(MemberKind::Struct)
	{}
};

struct AstBinOp
{
	static constexpr NodeKind nt = NodeKind::BinOp;

	BinOp op;
	AstNode* lhs;
	AstNode* rhs;

	AstBinOp(BinOp op, AstNode* lhs, AstNode* rhs)
		: op(op)
		, lhs(lhs)
		, rhs(rhs)
	{}
};

struct AstSwitch
{
	static constexpr NodeKind nt = NodeKind::Switch;

	AstNode* cond;
	std::vector<AstNode*> branches;

	AstSwitch(AstNode* cond, std::vector<AstNode*> branches)
		: cond(cond)
		, branches(branches)
	{}
};
struct AstCase
{
	static constexpr NodeKind nt = NodeKind::Case;

	AstNode* cond;
	AstNode* stmt;

	AstCase(AstNode* cond, AstNode* stmt)
		: cond(cond)
		, stmt(stmt)
	{}
};
struct AstBreak
{
	static constexpr NodeKind nt = NodeKind::Break;

	AstBreak() {}
};

struct AstDefault
{
	static constexpr NodeKind nt = NodeKind::Default;

	AstNode* stmt;

	AstDefault(AstNode* stmt)
		: stmt(stmt){};
};

struct AstFor
{
	static constexpr NodeKind nt = NodeKind::For;

	std::vector<AstNode*> inits;
	AstNode* cond;
	std::vector<AstNode*> afters;
	AstNode* body;

	AstFor(std::vector<AstNode*> inits, AstNode* cond, std::vector<AstNode*> afters, AstNode* body)
		: inits(inits)
		, cond(cond)
		, afters(afters)
		, body(body){};
};

struct AstWhile
{
	static constexpr NodeKind nt = NodeKind::While;

	AstNode* cond;
	AstNode* body;

	AstWhile(AstNode* cond, AstNode* body)
		: cond(cond)
		, body(body){};
};

struct AstAssign
{
	static constexpr NodeKind nt = NodeKind::Assign;

	AstNode* lhs;
	AstNode* rhs;

	AstAssign(AstNode* lhs, AstNode* rhs)
		: lhs(lhs)
		, rhs(rhs){};
};

struct AstNode
{
	Span span;
	NodeKind kind = NodeKind::Invalid;
	union NodeData
	{
		AstModule ast_module;
		AstBlock ast_block;
		AstFunc ast_func;
		AstFuncProto ast_func_proto;
		AstId ast_id;
		AstReturn ast_return;
		AstTypeDeclarator ast_type_declarator;
		AstVarDecl ast_var_decl;
		AstExpr ast_expr;
		AstStmt ast_stmt;
		AstNumberLiteral ast_number_literal;
		AstBoolLiteral ast_bool_literal;
		AstFuncCall ast_func_call;
		AstBinOp ast_bin_op;
		AstLet ast_let;
		AstIf ast_if;
		AstStruct ast_struct;
		AstUnion ast_union;
		AstEnum ast_enum;
		AstSizeOf ast_sizeof;
		AstAddressOf ast_addressof;
		AstEnumMember ast_enum_member;
		AstArrayAccess ast_array_access;
		AstMemberAccess ast_member_access;
		AstDeref ast_deref;
		AstBoolNot ast_boolnot;
		AstSwitch ast_switch;
		AstCase ast_case;
		AstDefault ast_default;
		AstBreak ast_break;
		AstFor ast_for;
		AstWhile ast_while;
		AstAssign ast_assign;

		// Attention! This leaks!
		NodeData() {}
		~NodeData() {}
	} data;
};
