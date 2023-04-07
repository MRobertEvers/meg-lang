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
	Block,
	Id,
	BinOp,
	NumberLiteral,
	TypeDeclarator,
	Return,
	VarDecl,
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

struct AstNumberLiteral
{
	static constexpr NodeKind nt = NodeKind::NumberLiteral;

	long long value;

	AstNumberLiteral(long long value)
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

struct AstBinOp
{
	static constexpr NodeKind nt = NodeKind::BinOp;

	BinOp op;
	AstNode* lhs;
	AstNode* rhs;

	AstBinOp(BinOp op, AstNode* lhs, AstNode* rhs)
		: lhs(lhs)
		, rhs(rhs)
	{}
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
		AstFuncCall ast_func_call;
		AstBinOp ast_bin_op;

		// Attention! This leaks!
		NodeData() {}
		~NodeData() {}
	} data;
};
