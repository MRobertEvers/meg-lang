#pragma once
#include "Span.h"
#include "bin_op.h"

#include <string>

enum class NodeKind
{
	Invalid,
	Module,
	Func,
	FuncProto,
	Block,
	Id,
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
	std::vector<std::string> name_parts;

	AstId(std::string name)
		: name_parts({name})
	{}

	AstId(std::vector<std::string> name_parts)
		: name_parts(name_parts)
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

struct AstNode
{
	Span span;
	NodeKind type = NodeKind::Invalid;
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

		// Attention! This leaks!
		NodeData() {}
		~NodeData() {}
	} data;
};