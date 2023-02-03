#pragma once
#include "CGResult.h"
#include "CGTag.h"
#include "CGed.h"
#include "ast2/Ast.h"
#include "ast2/AstNode.h"
#include "sema2/Scope.h"
#include "sema2/Sema2.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <map>
#include <memory>

namespace cg
{
class CG
{
	std::map<std::string, llvm::Function*> Functions;
	std::unique_ptr<llvm::LLVMContext> Context;
	std::unique_ptr<llvm::IRBuilder<>> Builder;

public:
	std::unique_ptr<llvm::Module> Module;
	using TagType = CGTag;

	std::map<sema::Scope*, CGScope> scopes;

	ast::Ast& ast;
	CG(ast::Ast& ast);

	CGResult<CGed> codegen(ast::AstNode* node);

	CGResult<CGed> codegen_module(ast::AstNode* node);
	CGResult<CGed> codegen_fn(ast::AstNode* node);
	CGResult<CGed> codegen_fn_proto(ast::AstNode* node);
	CGResult<CGed> codegen_id(ast::AstNode* node);
	CGResult<CGed> codegen_block(ast::AstNode* node);
	CGResult<CGed> codegen_fn_call(ast::AstNode* node);
	CGResult<CGed> codegen_fn_return(ast::AstNode* node);
	CGResult<CGed> codegen_number_literal(ast::AstNode* node);
	CGResult<CGed> codegen_expr(ast::AstNode* node);
	CGResult<CGed> codegen_stmt(ast::AstNode* node);

	CGScope* get_scope(ast::AstNode* node);
	sema::Scope* get_sema_scope(ast::AstNode* node);
};
} // namespace cg