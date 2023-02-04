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
	String* name;
	sema::Type const* type;
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
	Vec<IRStmt*>* stmts;
};

struct IRCall
{
	//
};

struct IRReturn
{
	//
	IRExpr* expr;
};

enum class IRStmtType
{
	Return,
};

struct IRStmt
{
	union
	{
		IRReturn* ret;
	} stmt;
};

enum class IRExprType
{
	Call,
};

struct IRExpr
{
	// Variant type
	union
	{
		IRCall* ret;
	} stmt;
};

}; // namespace ir