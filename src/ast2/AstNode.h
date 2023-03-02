#pragma once
#include "Span.h"
#include "bin_op.h"
#include "common/String.h"
#include "common/Vec.h"

namespace ast
{

enum class NodeType
{
	Invalid,
	Module,
	Fn,
	ExternFn,
	FnProto,
	FnParamList,
	ValueDecl,
	VarArg,
	FnCall,
	ArrayAccess,
	ExprList,
	Block,
	BinOp,
	Is,
	Id,
	Assign,
	If,
	IfArrow,
	Else,
	Let,
	Return,
	Struct,
	Namespace,
	Union,
	Initializer,
	InitializerDesignator,
	Switch,
	Case,
	Enum,
	EnumMember,
	// MemberDef,
	While,
	Yield,
	For,
	StringLiteral,
	NumberLiteral,
	TypeDeclarator,
	MemberAccess,
	IndirectMemberAccess,
	AddressOf,
	Deref,
	Empty,
	Expr,
	Stmt,
};

String to_string(NodeType type);

// Forward declare ast node.
struct AstNode;

template<typename T>
struct AstList
{
	Vec<T> list;

	void append(T elem) { list.push_back(elem); }
};

template<typename T>
T*
begin(AstList<T>* l)
{
	return l->list.begin().base();
}

template<typename T>
T*
end(AstList<T>* l)
{
	return l->list.end().base();
}

struct AstModule
{
	static constexpr NodeType nt = NodeType::Module;

	AstList<AstNode*>* statements;

	AstModule() = default;
	AstModule(AstList<AstNode*>* statements)
		: statements(statements)
	{}
};

struct AstFn
{
	static constexpr NodeType nt = NodeType::Fn;

	AstNode* prototype;
	AstNode* body;

	AstFn() = default;
	AstFn(AstNode* prototype, AstNode* body)
		: prototype(prototype)
		, body(body)
	{}
};

struct AstExternFn
{
	static constexpr NodeType nt = NodeType::ExternFn;

	AstNode* prototype;

	AstExternFn() = default;
	AstExternFn(AstNode* prototype)
		: prototype(prototype)
	{}
};

struct AstFnProto
{
	static constexpr NodeType nt = NodeType::FnProto;

	AstNode* name;
	AstNode* params;
	AstNode* return_type;

	AstFnProto() = default;
	AstFnProto(AstNode* name, AstNode* params, AstNode* return_type)
		: name(name)
		, params(params)
		, return_type(return_type)
	{}
};

struct AstFnParamList
{
	static constexpr NodeType nt = NodeType::FnParamList;

	AstList<AstNode*>* params;

	AstFnParamList() = default;
	AstFnParamList(AstList<AstNode*>* params)
		: params(params)
	{}
};

struct AstValueDecl
{
	static constexpr NodeType nt = NodeType::ValueDecl;

	AstNode* name;
	AstNode* type_name;

	AstValueDecl() = default;
	AstValueDecl(AstNode* name, AstNode* type_name)
		: name(name)
		, type_name(type_name)
	{}
};

struct AstEnumMember
{
	static constexpr NodeType nt = NodeType::EnumMember;

	enum class Type
	{
		Id,
		Struct
	} type;
	union
	{
		String* identifier;
		AstNode* struct_stmt;
	};

	AstEnumMember() = default;
	AstEnumMember(String* node)
		: type(Type::Id)
		, identifier(node)
	{}
	AstEnumMember(AstNode* node)
		: type(Type::Struct)
		, struct_stmt(node)
	{}
};

struct AstVarArg
{
	static constexpr NodeType nt = NodeType::VarArg;
};

struct AstEmpty
{
	static constexpr NodeType nt = NodeType::Empty;
};

struct AstFnCall
{
	static constexpr NodeType nt = NodeType::FnCall;

	AstNode* call_target;
	AstNode* args;

	AstFnCall() = default;
	AstFnCall(AstNode* call_target, AstNode* args)
		: call_target(call_target)
		, args(args)
	{}
};

struct AstArrayAccess
{
	static constexpr NodeType nt = NodeType::ArrayAccess;

	AstNode* array_target;
	AstNode* expr;

	AstArrayAccess() = default;
	AstArrayAccess(AstNode* array_target, AstNode* expr)
		: array_target(array_target)
		, expr(expr)
	{}
};

struct AstExprList
{
	static constexpr NodeType nt = NodeType::ExprList;

	AstList<AstNode*>* exprs;

	AstExprList() = default;
	AstExprList(AstList<AstNode*>* exprs)
		: exprs(exprs)
	{}
};

struct AstBlock
{
	static constexpr NodeType nt = NodeType::Block;

	AstList<AstNode*>* statements;

	AstBlock() = default;
	AstBlock(AstList<AstNode*>* statements)
		: statements(statements)
	{}
};

struct AstBinOp
{
	static constexpr NodeType nt = NodeType::BinOp;

	BinOp op;
	AstNode* left;
	AstNode* right;

	AstBinOp() = default;
	AstBinOp(BinOp op, AstNode* left, AstNode* right)
		: op(op)
		, left(left)
		, right(right)
	{}
};

struct AstIs
{
	static constexpr NodeType nt = NodeType::Is;

	AstNode* expr;
	AstNode* type_name;

	AstIs() = default;
	AstIs(AstNode* expr, AstNode* type_name)
		: expr(expr)
		, type_name(type_name)
	{}
};

enum IdClassification
{
	TypeIdentifier,
	ValueIdentifier
};

struct AstId
{
	static constexpr NodeType nt = NodeType::Id;

	IdClassification classification;
	AstList<String*>* name_parts;

	AstId() = default;
	AstId(IdClassification classification, AstList<String*>* name)
		: classification(classification)
		, name_parts(name)
	{}
};

enum class AssignOp
{
	assign,
	add,
	sub,
	mul,
	div
};

struct AstAssign
{
	static constexpr NodeType nt = NodeType::Assign;

	AssignOp op;
	AstNode* left;
	AstNode* right;

	AstAssign() = default;
	/// @brief
	/// @param op
	/// @param left
	/// @param right
	AstAssign(AssignOp op, AstNode* left, AstNode* right)
		: op(op)
		, left(left)
		, right(right)
	{}
};

struct AstIf
{
	static constexpr NodeType nt = NodeType::If;

	AstNode* condition;
	AstNode* then_block;
	AstNode* else_block;

	AstIf() = default;
	AstIf(AstNode* condition, AstNode* then_block, AstNode* else_block)
		: condition(condition)
		, then_block(then_block)
		, else_block(else_block)
	{}
};

struct AstIfArrow
{
	static constexpr NodeType nt = NodeType::IfArrow;

	AstNode* args;
	AstNode* block;

	AstIfArrow() = default;
	AstIfArrow(AstNode* args, AstNode* block)
		: args(args)
		, block(block)
	{}
};

struct AstElse
{
	static constexpr NodeType nt = NodeType::Else;

	AstNode* stmt;

	AstElse() = default;
	AstElse(AstNode* stmt)
		: stmt(stmt)
	{}
};

struct AstLet
{
	static constexpr NodeType nt = NodeType::Let;

	AstNode* identifier;
	AstNode* type_declarator;
	AstNode* rhs;

	AstLet() = default;
	AstLet(AstNode* identifier, AstNode* type_declarator, AstNode* rhs)
		: identifier(identifier)
		, type_declarator(type_declarator)
		, rhs(rhs)
	{}
};

struct AstReturn
{
	static constexpr NodeType nt = NodeType::Return;

	AstNode* expr;

	AstReturn() = default;
	AstReturn(AstNode* expr)
		: expr(expr)
	{}
};

struct AstStruct
{
	static constexpr NodeType nt = NodeType::Struct;

	AstNode* type_name;
	AstList<AstNode*>* members;

	AstStruct() = default;
	AstStruct(AstNode* type_name, AstList<AstNode*>* members)
		: type_name(type_name)
		, members(members)
	{}
};

struct AstNamespace
{
	static constexpr NodeType nt = NodeType::Namespace;

	AstNode* namespace_name;
	AstList<AstNode*>* statements;

	AstNamespace() = default;
	AstNamespace(AstNode* namespace_name, AstList<AstNode*>* members)
		: namespace_name(namespace_name)
		, statements(members)
	{}
};

struct AstUnion
{
	static constexpr NodeType nt = NodeType::Union;

	AstNode* type_name;
	AstList<AstNode*>* members;

	AstUnion() = default;
	AstUnion(AstNode* type_name, AstList<AstNode*>* members)
		: type_name(type_name)
		, members(members)
	{}
};

struct AstInitializerDesignator
{
	static constexpr NodeType nt = NodeType::InitializerDesignator;

	AstNode* name;
	AstNode* expr;

	AstInitializerDesignator() = default;
	AstInitializerDesignator(AstNode* name, AstNode* expr)
		: name(name)
		, expr(expr)
	{}
};

struct AstInitializer
{
	static constexpr NodeType nt = NodeType::Initializer;

	AstNode* type_name;
	AstList<AstNode*>* members;

	AstInitializer() = default;
	AstInitializer(AstNode* type_name, AstList<AstNode*>* members)
		: type_name(type_name)
		, members(members)
	{}
};

struct AstEnum
{
	static constexpr NodeType nt = NodeType::Enum;

	AstNode* type_name;
	AstList<AstNode*>* members;

	AstEnum() = default;
	AstEnum(AstNode* type_name, AstList<AstNode*>* members)
		: type_name(type_name)
		, members(members)
	{}
};

struct AstWhile
{
	static constexpr NodeType nt = NodeType::While;

	AstNode* condition;
	AstNode* block;

	AstWhile() = default;
	AstWhile(AstNode* condition, AstNode* block)
		: condition(condition)
		, block(block)
	{}
};

struct AstYield
{
	static constexpr NodeType nt = NodeType::Yield;

	AstNode* expr;

	AstYield() = default;
	AstYield(AstNode* expr)
		: expr(expr)
	{}
};

struct AstSwitch
{
	static constexpr NodeType nt = NodeType::Switch;

	AstNode* expr;
	AstNode* block;

	AstSwitch() = default;
	AstSwitch(AstNode* expr, AstNode* block)
		: expr(expr)
		, block(block)
	{}
};

struct AstCase
{
	static constexpr NodeType nt = NodeType::Case;

	AstNode* const_expr;
	bool is_default;
	AstNode* stmt;

	AstCase() = default;
	AstCase(AstNode* const_expr, AstNode* stmt)
		: const_expr(const_expr)
		, stmt(stmt)
		, is_default(false)
	{}
	AstCase(AstNode* stmt)
		: const_expr(nullptr)
		, stmt(stmt)
		, is_default(true)
	{}
};

struct AstFor
{
	static constexpr NodeType nt = NodeType::For;

	AstNode* init;
	AstNode* condition;
	AstNode* end_loop;
	AstNode* body;

	AstFor() = default;
	AstFor(AstNode* init, AstNode* condition, AstNode* end_loop, AstNode* body)
		: init(init)
		, condition(condition)
		, end_loop(end_loop)
		, body(body)
	{}
};

struct AstStringLiteral
{
	static constexpr NodeType nt = NodeType::StringLiteral;

	String* literal;

	AstStringLiteral() = default;
	AstStringLiteral(String* literal)
		: literal(literal)
	{}
};

struct AstNumberLiteral
{
	static constexpr NodeType nt = NodeType::NumberLiteral;

	long long literal;

	AstNumberLiteral() = default;
	AstNumberLiteral(long long literal)
		: literal(literal)
	{}
};

struct AstTypeDeclarator
{
	static constexpr NodeType nt = NodeType::TypeDeclarator;

	unsigned int array_size;
	unsigned int indirection_level;
	AstList<String*>* name;
	bool empty;
	bool is_impl;

	AstTypeDeclarator() = default;
	AstTypeDeclarator(AstList<String*>* name, unsigned int indirection_level, bool is_impl)
		: name(name)
		, indirection_level(indirection_level)
		, array_size(0)
		, empty(false)
		, is_impl(is_impl)
	{}
	AstTypeDeclarator(
		AstList<String*>* name, unsigned int indirection_level, unsigned array_size, bool is_impl)
		: name(name)
		, indirection_level(indirection_level)
		, array_size(array_size)
		, empty(false)
		, is_impl(is_impl)
	{}
};

struct AstMemberAccess
{
	static constexpr NodeType nt = NodeType::MemberAccess;

	AstNode* expr;
	AstNode* member_name;

	AstMemberAccess() = default;
	AstMemberAccess(AstNode* expr, AstNode* member_name)
		: expr(expr)
		, member_name(member_name)
	{}
};

struct AstIndirectMemberAccess
{
	static constexpr NodeType nt = NodeType::IndirectMemberAccess;

	AstNode* expr;
	AstNode* member_name;

	AstIndirectMemberAccess() = default;
	AstIndirectMemberAccess(AstNode* expr, AstNode* member_name)
		: expr(expr)
		, member_name(member_name)
	{}
};

struct AstAddressOf
{
	static constexpr NodeType nt = NodeType::AddressOf;

	AstNode* expr;

	AstAddressOf() = default;
	AstAddressOf(AstNode* expr)
		: expr(expr)
	{}
};

// This is '*'
struct AstDeref
{
	static constexpr NodeType nt = NodeType::Deref;

	AstNode* expr;

	AstDeref() = default;
	AstDeref(AstNode* expr)
		: expr(expr)
	{}
};

struct AstExpr
{
	static constexpr NodeType nt = NodeType::Expr;

	AstNode* expr;

	AstExpr() = default;
	AstExpr(AstNode* expr)
		: expr(expr)
	{}
};

struct AstStmt
{
	static constexpr NodeType nt = NodeType::Stmt;

	AstNode* stmt;

	AstStmt() = default;
	AstStmt(AstNode* stmt)
		: stmt(stmt)
	{}
};

struct AstNode
{
	Span span;
	NodeType type = NodeType::Invalid;
	union
	{
		AstModule mod;
		AstNamespace namespace_node;
		AstFn fn;
		AstExternFn extern_fn;
		AstFnProto fn_proto;
		AstFnParamList fn_params;
		AstValueDecl value_decl;
		AstVarArg var_arg;
		AstFnCall fn_call;
		AstArrayAccess array_access;
		AstExprList expr_list;
		AstBlock block;
		AstBinOp binop;
		AstId id;
		AstAssign assign;
		AstIs is;
		AstInitializer initializer;
		AstInitializerDesignator designator;
		AstIf ifcond;
		AstSwitch switch_stmt;
		AstCase case_stmt;
		AstYield yield;
		AstIfArrow if_arrow;
		AstElse else_stmt;
		AstLet let;
		AstReturn returnexpr;
		AstStruct structstmt;
		AstUnion unionstmt;
		AstEnum enumstmt;
		AstEnumMember enum_member;
		// AstMemberDef member;
		AstWhile whilestmt;
		AstFor forstmt;
		AstStringLiteral string_literal;
		AstNumberLiteral number_literal;
		AstTypeDeclarator type_declarator;
		AstMemberAccess member_access;
		AstIndirectMemberAccess indirect_member_access;
		AstDeref deref;
		AstEmpty empty;
		AstAddressOf address_of;
		AstExpr expr;
		AstStmt stmt;
	} data;
};

} // namespace ast