#pragma once

#include "Expr.h"
#include "Function.h"
#include "sema3/HirNode.h"
#include "sema3/Sym.h"
#include "sema3/SymBuiltins.h"
#include "sema3/Ty.h"
#include <llvm/IR/IRBuilder.h>

#include <map>
#include <memory>

class Codegen
{
private:
	std::unique_ptr<llvm::LLVMContext> context;
	std::unique_ptr<llvm::IRBuilder<>> builder;
	std::unique_ptr<llvm::Module> mod;

	std::map<Sym*, Address> vars;
	std::map<Sym*, Function> funcs;
	std::map<Ty const*, llvm::Type*> tys;

public:
	Codegen(SymBuiltins& builtins);

	void print();

	Expr codegen_module(HirNode*);
	Expr codegen_item(HirNode*);
	Expr codegen_function(HirNode*);
	Expr codegen_call(HirNode*);
	Expr codegen_binop(HirNode* hir_call);
	Expr codegen_builtin(HirNode* hir_call);
	Expr codegen_intcast(HirNode* hir_call);
	Expr codegen_block(HirNode*);
	Expr codegen_var(HirNode*);
	Expr codegen_statement(HirNode*);
	Expr codegen_expr(HirNode*);
	Expr codegen_return(HirNode*);
	Expr codegen_number_literal(HirNode*);

	llvm::Value* codegen_eval(Expr expr);
	llvm::Type* get_type(QualifiedTy qty);
	llvm::Type* get_type(Ty const* ty);
	static Codegen codegen(SymBuiltins& builtins, HirNode* module);
};
