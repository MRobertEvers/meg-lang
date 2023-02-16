#pragma once
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
struct IRExpr;
struct IRAssign;
struct IRStruct;
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
	Struct
};

struct IRTopLevelStmt
{
	ast::AstNode* node;
	//
	union
	{
		IRStruct* struct_decl;
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

struct IRId
{
	//
	String* name;
	sema::TypeInstance type_instance;
	ast::AstNode* node;
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

struct IRMemberAccess
{
	//
	ast::AstNode* node;
	String* member_name;
	sema::TypeInstance type_instance;
	IRExpr* expr;
};

struct IRIndirectMemberAccess
{
	//
	ast::AstNode* node;
	String* member_name;
	sema::TypeInstance type_instance;
	IRExpr* expr;
};

struct IRIf
{
	//
	ast::AstNode* node;
	IRExpr* expr;
	IRStmt* stmt;
	IRElse* else_stmt;
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
	Else,
	Block,
};

struct IRStmt
{
	ast::AstNode* node;
	union
	{
		IRIf* if_stmt;
		IRElse* else_stmt;
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
	NumberLiteral,
	StringLiteral,
	Id,
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
		IRBinOp* binop;
		IRAddressOf* addr_of;
		IRDeref* deref;
		IREmpty* empty;
		IRMemberAccess* member_access;
		IRIndirectMemberAccess* indirect_member_access;
	} expr;
	IRExprType type;

	sema::TypeInstance type_instance;
};

}; // namespace ir