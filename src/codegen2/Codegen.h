#pragma once
#include "CGExpr.h"
#include "CGResult.h"
#include "Codegen/CGFunctionContext.h"
#include "Scope.h"
#include "ast2/Ast.h"
#include "ast2/AstNode.h"
#include "common/String.h"
#include "sema2/IR.h"
#include "sema2/Scope.h"
#include "sema2/Sema2.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <map>
#include <memory>
#include <optional>

namespace cg
{

class CG
{
public:
	std::map<std::string, llvm::Function*> Functions;
	std::unique_ptr<llvm::LLVMContext> Context;
	std::unique_ptr<llvm::IRBuilder<>> Builder;

	std::optional<CGFunctionContext> current_function;

	std::unique_ptr<llvm::Module> Module;
	// TODO: Need scoping on these types.
	std::map<sema::Type const*, llvm::Type*> types;
	std::map<String, CGExpr> values;

	// Vec<cg::Scope> scopes;
	// Scope* current_scope;

	sema::Sema2& sema;
	CG(sema::Sema2& sema);

	// Scope* push_scope();
	// void pop_scope();

	void add_function(String const& name, llvm::Function*, llvm::Type*, sema::Type const*);

	CGResult<CGExpr> codegen_module(ir::IRModule*);
	CGResult<CGExpr> codegen_tls(ir::IRTopLevelStmt*);
	CGResult<CGExpr> codegen_stmt(ir::IRStmt*);
	CGResult<CGExpr> codegen_expr(ir::IRExpr*);
	CGResult<CGExpr> codegen_expr(ir::IRExpr*, std::optional<CGExpr>);
	CGResult<CGExpr> codegen_extern_fn(ir::IRExternFn*);
	CGResult<CGExpr> codegen_return(ir::IRReturn*);
	CGResult<CGExpr> codegen_member_access(ir::IRMemberAccess*);
	CGResult<CGExpr> codegen_let(ir::IRLet*);
	CGResult<CGExpr> codegen_assign(ir::IRAssign*);

	// TODO: Return RValue type?
	CGResult<CGExpr> codegen_number_literal(ir::IRNumberLiteral*);
	CGResult<CGExpr> codegen_string_literal(ir::IRStringLiteral*);
	CGResult<CGExpr> codegen_value_decl(ir::IRValueDecl*);
	CGResult<CGExpr> codegen_binop(ir::IRBinOp*);
	CGResult<CGExpr> codegen_call(ir::IRCall*, std::optional<CGExpr>);
	CGResult<CGExpr> codegen_id(ir::IRId*);
	CGResult<CGExpr> codegen_struct(ir::IRStruct* st);

	std::optional<llvm::Type*> find_type(sema::Type const*);
};

} // namespace cg