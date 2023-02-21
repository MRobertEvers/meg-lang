#pragma once
#include "MemberTypeInstance.h"
#include "TypeInstance.h"
#include "ast2/AstNode.h"
#include "common/String.h"
#include "common/Vec.h"
#include "type/Type.h"

#include <map>

namespace ir
{
struct IRModule;
struct IRTopLevelStmt;
struct IRTypeDeclaraor;
struct IRFunction;
struct IRExternFn;
struct IRProto;
struct IRBlock;
struct IRCall;
struct IRStmt;
struct IRExpr;
struct IRValueDecl;
struct IREnum;
struct IREnumMember;
struct IRExpr;
struct IRAssign;
struct IRStruct;
struct IRUnion;
struct IRParam;
struct IRElse;

struct IRModule
{
	//
	ast::AstNode* node;
	Vec<IRTopLevelStmt*>* stmts;
};

enum IRTopLevelType
{
	Function,
	ExternFn,
	Struct,
	Union,
	Enum,
};

struct IRTopLevelStmt
{
	ast::AstNode* node;
	//
	union
	{
		IRUnion* union_decl;
		IRStruct* struct_decl;
		IREnum* enum_decl;
		IRFunction* fn;
		IRExternFn* extern_fn;
	} stmt;

	IRTopLevelType type;
};

struct IRFunction
{
	ast::AstNode* node;
	//
	IRProto* proto;
	IRBlock* block;
};

struct IRExternFn
{
	ast::AstNode* node;
	IRProto* proto;
};

struct IRProto
{
	//
	ast::AstNode* node;
	String* name;
	Vec<ir::IRParam*>* args;
	ir::IRTypeDeclaraor* rt;

	// Function
	sema::Type const* fn_type;
};

struct IRValueDecl
{
	//
	ast::AstNode* node;
	IRTypeDeclaraor* type_decl;
	String* name;
};

struct IRVarArg
{
	ast::AstNode* node;
};

enum class IRParamType
{
	VarArg,
	ValueDecl
};

struct IRParam
{
	ast::AstNode* node;
	union
	{
		IRValueDecl* value_decl;
		IRVarArg* var_arg;
	} data;
	IRParamType type;
};

struct IRTypeDeclaraor
{
	ast::AstNode* node;
	sema::TypeInstance type_instance;
};

struct IRStruct
{
	ast::AstNode* node;
	// TODO: Support non-member decls.
	std::map<String, ir::IRValueDecl*>* members;

	sema::Type const* struct_type;
};

struct IRUnion
{
	ast::AstNode* node;
	// TODO: Support non-member decls.
	std::map<String, ir::IRValueDecl*>* members;

	sema::Type const* union_type;
};

struct IREnum
{
	ast::AstNode* node;
	std::map<String, ir::IREnumMember*>* members;

	sema::Type const* enum_type;
};

struct IRBlock
{
	//
	ast::AstNode* node;
	Vec<IRStmt*>* stmts;
};

struct IRArgs
{
	//
	ast::AstNode* node;
	Vec<ir::IRExpr*>* args;
};

struct IRCall
{
	//
	IRExpr* call_target;
	IRArgs* args;

	ast::AstNode* node;
};

struct IRArrayAccess
{
	//
	IRExpr* array_target;
	IRExpr* expr;
	sema::TypeInstance type_instance;

	ast::AstNode* node;
};

struct IRId
{
	//
	Vec<String*>* name;
	sema::TypeInstance type_instance;
	ast::AstNode* node;

	bool is_type_id;
};

struct IRReturn
{
	//
	ast::AstNode* node;
	IRExpr* expr;
};

struct IRLet
{
	//
	ast::AstNode* node;
	String* name;
	IRAssign* assign;
	sema::TypeInstance type_instance;

	bool is_empty() const { return assign == nullptr; }
};

struct IRAssign
{
	//
	ast::AssignOp op;
	ast::AstNode* node;
	IRExpr* lhs;
	IRExpr* rhs;
};

struct IRBinOp
{
	ast::BinOp op;
	ast::AstNode* node;
	sema::TypeInstance type_instance;
	IRExpr* lhs;
	IRExpr* rhs;
};

struct IRIs
{
	ast::AstNode* node;
	// This is always BOOL TYPE!
	sema::TypeInstance type_instance;

	// If you want the checked type, look here.
	IRTypeDeclaraor* type_decl;
	IRExpr* lhs;
};

struct IRNumberLiteral
{
	//
	ast::AstNode* node;
	long long val;
	sema::TypeInstance type_instance;
};

struct IRStringLiteral
{
	//
	ast::AstNode* node;
	String* value;
	sema::TypeInstance type_instance;
};

struct IRDesignator
{
	ast::AstNode* node;
	sema::MemberTypeInstance member;
	ir::IRExpr* expr;
};

struct IRInitializer
{
	//
	ast::AstNode* node;
	String* name;
	Vec<IRDesignator*>* initializers;
	sema::TypeInstance type_instance;
};

struct IRMemberAccess
{
	//
	ast::AstNode* node;
	String* member_name;
	sema::MemberTypeInstance member;
	IRExpr* expr;
};

struct IRIndirectMemberAccess
{
	//
	ast::AstNode* node;
	String* member_name;
	sema::MemberTypeInstance member;
	IRExpr* expr;
};

struct IREnumMember
{
	ast::AstNode* node;
	sema::EnumNominal number;
	enum class Type
	{
		Id,
		Struct,
	} contained_type;

	union
	{
		IRStruct* struct_member;
	};

	String* name;
	sema::Type const* type;
};

struct IRIf
{
	//
	ast::AstNode* node;
	IRExpr* expr;
	IRStmt* stmt;
	IRElse* else_stmt;

	// For if arrow
	Vec<ir::IRParam*>* discriminations;
};

struct IRFor
{
	//
	ast::AstNode* node;
	IRExpr* condition;
	IRStmt* init;
	IRStmt* end;
	IRStmt* body;
};

struct IRWhile
{
	//
	ast::AstNode* node;
	IRExpr* condition;
	IRStmt* body;
};

struct IRElse
{
	//
	ast::AstNode* node;
	IRStmt* stmt;
};

struct IRAddressOf
{
	//
	ast::AstNode* node;
	IRExpr* expr;
	sema::TypeInstance type_instance;
};

struct IRDeref
{
	//
	ast::AstNode* node;
	IRExpr* expr;
	sema::TypeInstance type_instance;
};

struct IREmpty
{
	//
	ast::AstNode* node;
	sema::TypeInstance type_instance; // Should be void type
};

enum class IRStmtType
{
	Return,
	ExprStmt,
	Let,
	Assign,
	If,
	For,
	While,
	Else,
	Block,
};

struct IRStmt
{
	ast::AstNode* node;
	union
	{
		IRIf* if_stmt;
		IRFor* for_stmt;
		IRElse* else_stmt;
		IRWhile* while_stmt;
		IRExpr* expr;
		IRBlock* block;
		IRReturn* ret;
		IRLet* let;
		IRAssign* assign;
	} stmt;
	IRStmtType type;
};

enum class IRExprType
{
	Call,
	ArrayAccess,
	NumberLiteral,
	StringLiteral,
	Id,
	Is,
	Initializer,
	BinOp,
	ValueDecl,
	MemberAccess,
	IndirectMemberAccess,
	AddressOf,
	Empty,
	Deref
};

struct IRExpr
{
	ast::AstNode* node;
	// Variant type
	union
	{
		IRId* id;		   // Usage of an id
		IRValueDecl* decl; // Variable declaration
		IRStringLiteral* str_literal;
		IRNumberLiteral* num_literal;
		IRCall* call;
		IRArrayAccess* array_access;
		IRBinOp* binop;
		IRIs* is;
		IRInitializer* initializer;
		IRAddressOf* addr_of;
		IRDeref* deref;
		IREmpty* empty;
		IRMemberAccess* member_access;
		IRIndirectMemberAccess* indirect_member_access;
	} expr;
	IRExprType type;

	Vec<IRIs*>* discriminations;

	sema::TypeInstance type_instance;
};

}; // namespace ir