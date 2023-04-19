#pragma once

#include "Expr.h"
#include "sema3/HirNode.h"
#include <llvm/IR/IRBuilder.h>

#include <memory>

class Codegen
{
private:
	std::unique_ptr<llvm::LLVMContext> context;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::Module> mod;

public:
	Codegen();

	void print();

	Expr codegen_module(HirNode*);
	Expr codegen_item(HirNode*);
	Expr codegen_function(HirNode*);
	Expr codegen_block(HirNode*);
	Expr codegen_statement(HirNode*);
	Expr codegen_expr(HirNode*);
	Expr codegen_return(HirNode*);
	Expr codegen_number_literal(HirNode*);

	llvm::Value* codegen_eval(Expr expr);
	static Codegen codegen(HirNode* module);
};
