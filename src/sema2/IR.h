#pragma once
#include "TypeInstance.h"
#include "ast2/AstNode.h"
#include "common/String.h"
#include "common/Vec.h"
#include "type/Type.h"

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

struct IRModule
{
	//
	ast::AstNode* node;
	Vec<IRTopLevelStmt*>* stmts;
};

enum IRTopLevelType
{
	Function,
	ExternFn
};

struct IRTopLevelStmt
{
	ast::AstNode* node;
	//
	union
	{
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
	Vec<ir::IRValueDecl*>* args;
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

struct IRTypeDeclaraor
{
	ast::AstNode* node;
	sema::TypeInstance type_instance;
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

struct IRNumberLiteral
{
	//
	ast::AstNode* node;
	long long val;
};

struct IRStringLiteral
{
	//
	ast::AstNode* node;
	String* value;
};

enum class IRStmtType
{
	Return,
	ExprStmt,
};

struct IRStmt
{
	ast::AstNode* node;
	union
	{
		IRExpr* expr;
		IRReturn* ret;
	} stmt;
	IRStmtType type;
};

enum class IRExprType
{
	Call,
	NumberLiteral,
	StringLiteral,
	Id,
};

struct IRExpr
{
	ast::AstNode* node;
	// Variant type
	union
	{
		IRId* id;
		IRStringLiteral* str_literal;
		IRNumberLiteral* num_literal;
		IRCall* call;
	} expr;
	IRExprType type;
};

}; // namespace ir