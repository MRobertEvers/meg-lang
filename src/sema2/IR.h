#pragma once
#include "TypeInstance.h"
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
	Vec<IRTopLevelStmt*>* stmts;
};

enum IRTopLevelType
{
	Function,
	Prototype
};

struct IRTopLevelStmt
{
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
	//
	IRProto* proto;
	IRBlock* block;
};

struct IRExternFn
{
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
	IRTypeDeclaraor* type_decl;
	String* name;
};

struct IRTypeDeclaraor
{
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