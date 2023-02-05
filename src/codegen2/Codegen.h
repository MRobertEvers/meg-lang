#pragma once
#include "CGResult.h"
#include "CGTag.h"
#include "CGed.h"
#include "ast2/Ast.h"
#include "ast2/AstNode.h"
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

	std::unique_ptr<llvm::Module> Module;
	std::map<sema::Type const*, llvm::Type*> types;
	using TagType = CGTag;

	std::map<sema::Scope*, CGScope> scopes;

	sema::Sema2& sema;
	CG(sema::Sema2& sema);

	// CGResult<CGed> codegen(ast::AstNode* node);

	CGResult<CGExpr> codegen_module(ir::IRModule*);
	CGResult<CGExpr> codegen_tls(ir::IRTopLevelStmt*);
	CGResult<CGExpr> codegen_extern_fn(ir::IRExternFn*);
	// CGResult<CGed> codegen_fn_proto(ast::AstNode* node);
	// CGResult<CGed> codegen_id(ast::AstNode* node);
	// CGResult<CGed> codegen_block(ast::AstNode* node);
	// CGResult<CGed> codegen_fn_call(ast::AstNode* node);
	// CGResult<CGed> codegen_fn_return(ast::AstNode* node);
	// CGResult<CGed> codegen_number_literal(ast::AstNode* node);
	// CGResult<CGed> codegen_expr(ast::AstNode* node);
	// CGResult<CGed> codegen_stmt(ast::AstNode* node);

	// CGScope* get_scope(ast::AstNode* node);
	// sema::Scope* get_sema_scope(ast::AstNode* node);

	std::optional<llvm::Type*> find_type(sema::Type const*);
};

} // namespace cg