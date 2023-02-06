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
	ExprList,
	Block,
	BinOp,
	Id,
	Assign,
	If,
	Let,
	Return,
	Struct,
	// MemberDef,
	While,
	For,
	StringLiteral,
	NumberLiteral,
	TypeDeclarator,
	MemberAccess,
	Expr,
	Stmt,
};

String to_string(NodeType type);

// Forward declare ast node.
struct AstNode;

template<typename T>
struct AstList
{
	Vec<AstNode*> list;

	void append(AstNode* elem) { list.push_back(elem); }
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

struct AstVarArg
{
	static constexpr NodeType nt = NodeType::VarArg;
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

enum IdClassification
{
	TypeIdentifier,
	ValueIdentifier
};

struct AstId
{
	static constexpr NodeType nt = NodeType::Id;

	IdClassification classification;
	String* name;

	AstId() = default;
	AstId(IdClassification classification, String* name)
		: classification(classification)
		, name(name)
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

// struct AstMemberDef
// {
// 	static constexpr NodeType nt = NodeType::MemberDef;

// 	AstNode* identifier;
// 	AstNode* type_declarator;

// 	AstMemberDef() = default;
// 	AstMemberDef(AstNode* identifier, AstNode* type_declarator)
// 		: identifier(identifier)
// 		, type_declarator(type_declarator)
// 	{}
// };

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

	unsigned int indirection_level;
	String* name;
	bool empty;

	AstTypeDeclarator() = default;
	AstTypeDeclarator(String* name, unsigned int indirection_level)
		: name(name)
		, indirection_level(indirection_level)
		, empty(false)
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
		AstFn fn;
		AstExternFn extern_fn;
		AstFnProto fn_proto;
		AstFnParamList fn_params;
		AstValueDecl value_decl;
		AstVarArg var_arg;
		AstFnCall fn_call;
		AstExprList expr_list;
		AstBlock block;
		AstBinOp binop;
		AstId id;
		AstAssign assign;
		AstIf ifcond;
		AstLet let;
		AstReturn returnexpr;
		AstStruct structstmt;
		// AstMemberDef member;
		AstWhile whilestmt;
		AstFor forstmt;
		AstStringLiteral string_literal;
		AstNumberLiteral number_literal;
		AstTypeDeclarator type_declarator;
		AstMemberAccess member_access;
		AstExpr expr;
		AstStmt stmt;
	} data;
};

} // namespace ast