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

	Function* current_func = nullptr;
	std::map<Sym*, Address> vars;
	std::map<Sym*, Function> funcs;
	std::map<Ty const*, llvm::Type*> tys;

public:
	Codegen(SymBuiltins& builtins);

	void print();
	int emit();

	Expr codegen_module(HirNode*);
	Expr codegen_item(HirNode*);
	Expr codegen_func(HirNode*);
	Function* codegen_func_proto(HirNode*);
	Expr codegen_call(HirNode*);
	Expr codegen_func_call(HirNode*);
	Expr codegen_binop(HirNode* hir_call);
	Expr codegen_builtin(HirNode* hir_call);
	Expr codegen_intcast(HirNode* hir_call);
	Expr codegen_block(HirNode*);
	Expr codegen_if(HirNode*);
	Expr codegen_if_chain(HirNode*);
	Expr codegen_if_phi(HirNode*);
	Expr codegen_var(HirNode*);
	Expr codegen_expr(HirNode*);
	Expr codegen_return(HirNode*);
	Expr codegen_number_literal(HirNode*);
	Expr codegen_string_literal(HirNode*);

	llvm::Value* codegen_memcpy(Expr expr, llvm::Value* dest);
	llvm::Value* codegen_eval(Expr expr);
	llvm::Type* get_type(QualifiedTy qty);
	llvm::Type* get_type(Ty const* ty);
	static Codegen codegen(SymBuiltins& builtins, HirNode* module);
};
