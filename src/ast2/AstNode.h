#include "Span.h"
#include "common/String.h"
#include "common/Vec.h"

#include <variant>

namespace ast
{

enum class NodeType
{
	Invalid,
	Fn,
	FnProto,
	FnCall,
	Block,
	BinOp,
	Id,
	Assign,
	If,
	Let,
	Return,
	Struct,
	While,
	StringLiteral,
	NumberLiteral,
	TypeDeclarator,
	MemberAccess,
};

// Forward declare ast node.
struct AstNode;
struct Ast;

template<typename T>
struct AstList
{
	Vec<AstNode*> list;

	void append(AstNode* elem) { list.push_back(elem); }
};

struct AstFn
{
	static constexpr NodeType type = NodeType::Fn;

	AstNode* prototype;
	AstNode* body;

	AstFn() = default;
	AstFn(AstNode* prototype, AstNode* body)
		: prototype(prototype)
		, body(body)
	{}
};

struct AstFnProto
{
	static constexpr NodeType type = NodeType::FnProto;

	AstList<AstNode*>* params;
	AstNode* return_type;

	AstFnProto() = default;
	AstFnProto(AstList<AstNode*>* params, AstNode* return_type)
		: params(params)
		, return_type(return_type)
	{}
};

struct AstFnCall
{
	static constexpr NodeType type = NodeType::FnCall;

	AstList<AstNode*>* args;

	AstFnCall() = default;
	AstFnCall(AstList<AstNode*>* args)
		: args(args)
	{}
};

struct AstBlock
{
	static constexpr NodeType type = NodeType::Block;

	AstList<AstNode*>* statements;

	AstBlock() = default;
	AstBlock(AstList<AstNode*>* statements)
		: statements(statements)
	{}
};

enum BinOp : char
{
	plus,
	star,
	minus,
	slash,
	gt,
	gte,
	lt,
	lte,
	and_lex,
	or_lex,
	cmp,
	ne,
	bad
};

struct AstBinOp
{
	static constexpr NodeType type = NodeType::BinOp;

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

struct AstId
{
	static constexpr NodeType type = NodeType::Id;

	String* name;

	AstId() = default;
	AstId(String* name)
		: name(name)
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
	static constexpr NodeType type = NodeType::Assign;

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
	static constexpr NodeType type = NodeType::If;

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
	static constexpr NodeType type = NodeType::Let;

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
	static constexpr NodeType type = NodeType::Return;

	AstNode* expr;

	AstReturn() = default;
	AstReturn(AstNode* expr)
		: expr(expr)
	{}
};

struct AstStruct
{
	static constexpr NodeType type = NodeType::Struct;

	AstNode* type_name;
	AstList<AstNode*>* members;

	AstStruct() = default;
	AstStruct(AstNode* type_name, AstList<AstNode*>* members)
		: type_name(type_name)
		, members(members)
	{}
};

struct AstWhile
{
	static constexpr NodeType type = NodeType::While;

	AstNode* condition;
	AstNode* block;

	AstWhile() = default;
	AstWhile(AstNode* condition, AstNode* block)
		: condition(condition)
		, block(block)
	{}
};

struct AstStringLiteral
{
	static constexpr NodeType type = NodeType::StringLiteral;

	String* literal;

	AstStringLiteral() = default;
	AstStringLiteral(String* literal)
		: literal(literal)
	{}
};

struct AstNumberLiteral
{
	static constexpr NodeType type = NodeType::NumberLiteral;

	unsigned long long literal;

	AstNumberLiteral() = default;
	AstNumberLiteral(unsigned long long literal)
		: literal(literal)
	{}
};

struct AstTypeDeclarator
{
	static constexpr NodeType type = NodeType::TypeDeclarator;

	unsigned int indirection_level;
	String* name;

	AstTypeDeclarator() = default;
	AstTypeDeclarator(String* name, unsigned int indirection_level)
		: name(name)
		, indirection_level(indirection_level)
	{}
};

struct AstMemberAccess
{
	static constexpr NodeType type = NodeType::MemberAccess;

	AstNode* expr;
	AstNode* member_name;

	AstMemberAccess() = default;
	AstMemberAccess(AstNode* expr, AstNode* member_name)
		: expr(expr)
		, member_name(member_name)
	{}
};

struct AstNode
{
	Span span;
	NodeType type = NodeType::Invalid;
	union
	{
		AstFn fn;
		AstFnProto fn_proto;
		AstFnCall fn_call;
		AstBlock block;
		AstBinOp binop;
		AstId id;
		AstAssign assign;
		AstIf ifcond;
		AstLet let;
		AstReturn returnexpr;
		AstStruct structstmt;
		AstWhile whilestmt;
		AstStringLiteral string_literal;
		AstNumberLiteral number_literal;
		AstTypeDeclarator type_declarator;
		AstMemberAccess member_access;
	} data;
};

// TODO: AST owns all ast nodes.
// Track all memory allocs
struct Ast
{
	String* create_string(char const* cstr, unsigned int size) { return new String{cstr, size}; }
	AstList<AstNode*>* create_list() { return new AstList<AstNode*>{}; }

	template<typename T>
	AstNode* make_empty(Span span)
	{
		auto node = new AstNode;
		node->type = T::type;
		node->span = span;

		return node;
	}

	AstNode* Fn(Span span, AstNode* prototype, AstNode* body)
	{
		auto node = make_empty<AstFn>(span);
		node->data.fn = AstFn{prototype, body};
		return node;
	}

	AstNode* FnProto(Span span, AstList<AstNode*>* params, AstNode* return_type)
	{
		auto node = make_empty<AstFnProto>(span);
		node->data.fn_proto = AstFnProto{params, return_type};
		return node;
	}

	AstNode* FnCall(Span span, AstList<AstNode*>* args)
	{
		auto node = make_empty<AstFnCall>(span);
		node->data.fn_call = AstFnCall{args};
		return node;
	}

	AstNode* Block(Span span, AstList<AstNode*>* statements)
	{
		auto node = make_empty<AstBlock>(span);
		node->data.block = AstBlock{statements};
		return node;
	}

	AstNode* BinOp(Span span, BinOp op, AstNode* left, AstNode* right)
	{
		auto node = make_empty<AstBinOp>(span);
		node->data.binop = AstBinOp{op, left, right};
		return node;
	}

	AstNode* Id(Span span, String* name)
	{
		auto node = make_empty<AstId>(span);
		node->data.id = AstId{name};
		return node;
	}

	AstNode* Assign(Span span, AssignOp op, AstNode* left, AstNode* right)
	{
		auto node = make_empty<AstAssign>(span);
		node->data.assign = AstAssign{op, left, right};
		return node;
	}

	AstNode* If(Span span, AstNode* condition, AstNode* then_block, AstNode* else_block)
	{
		auto node = make_empty<AstIf>(span);
		node->data.ifcond = AstIf{condition, then_block, else_block};
		return node;
	}

	AstNode* Let(Span span, AstNode* identifier, AstNode* type_declarator, AstNode* rhs)
	{
		auto node = make_empty<AstLet>(span);
		node->data.let = AstLet{identifier, type_declarator, rhs};
		return node;
	}

	AstNode* Return(Span span, AstNode* expr)
	{
		auto node = make_empty<AstReturn>(span);
		node->data.returnexpr = AstReturn{expr};
		return node;
	}

	AstNode* Struct(Span span, AstNode* type_name, AstList<AstNode*>* members)
	{
		auto node = make_empty<AstStruct>(span);
		node->data.structstmt = AstStruct{type_name, members};
		return node;
	}

	AstNode* While(Span span, AstNode* condition, AstNode* block)
	{
		auto node = make_empty<AstWhile>(span);
		node->data.whilestmt = AstWhile{condition, block};
		return node;
	}

	AstNode* StringLiteral(Span span, String* literal)
	{
		auto node = make_empty<AstStringLiteral>(span);
		node->data.string_literal = AstStringLiteral{literal};
		return node;
	}

	AstNode* NumberLiteral(Span span, unsigned long long literal)
	{
		auto node = make_empty<AstNumberLiteral>(span);
		node->data.number_literal = AstNumberLiteral{literal};
		return node;
	}

	AstNode* TypeDeclarator(Span span, String* name, unsigned int indirection_level)
	{
		auto node = make_empty<AstTypeDeclarator>(span);
		node->data.type_declarator = AstTypeDeclarator{name, indirection_level};
		return node;
	}

	AstNode* TypeDeclarator(Span span, AstNode* expr, AstNode* member_name)
	{
		auto node = make_empty<AstMemberAccess>(span);
		node->data.member_access = AstMemberAccess{expr, member_name};
		return node;
	}
};

} // namespace ast