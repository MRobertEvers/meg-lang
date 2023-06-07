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
	Expr codegen_struct(HirNode*);
	Expr codegen_func(HirNode*);
	void codegen_func_entry(Function* func);
	Expr codegen_sync_func(HirNode*);
	Expr codegen_async_func(HirNode*);
	Function* codegen_sync_proto(HirNode*);

	/**
	 * codegen_async_constructor create the async frame which is used in the rest of the functions.
	 * codegen_async_begin, and codegen_async_send, call into codegen_async_step.
	 * codegen_async_begin takes no arguments.
	 * Async step takes an "optional" struct. It's because the first step doesn't have a yield
	 * value.
	 *
	 * @return Expr
	 */
	Expr codegen_async_constructor(HirNode*, llvm::Type* frame);
	struct codegen_async_step_t
	{
		llvm::Function* step_fn;
		llvm::Type* send_opt_ty;
		llvm::Type* send_ty;
		llvm::Type* iter_ty;
	};
	codegen_async_step_t codegen_async_step(HirNode*, llvm::Type* frame);
	Expr codegen_async_begin(HirNode*, llvm::Type* frame, codegen_async_step_t step);
	Expr codegen_async_send(HirNode*, llvm::Type* frame, codegen_async_step_t step);
	Expr codegen_async_close(HirNode*, llvm::Type* frame);
	llvm::Type* codegen_async_frame(HirNode*);

	Expr codegen_construct(HirNode*);
	Expr codegen_func_call(HirNode*, Expr sret);
	Expr codegen_func_call_static(HirNode*, Expr sret);
	Expr codegen_binop(HirNode* hir_call);
	Expr codegen_builtin(HirNode* hir_call);
	Expr codegen_intcast(HirNode* hir_call);
	Expr codegen_deref(HirNode* hir_call);
	Expr codegen_addressof(HirNode* hir_call);
	Expr codegen_member_access(HirNode*);
	Expr codegen_block(HirNode*);
	Expr codegen_let(HirNode*);
	Expr codegen_switch(HirNode*);
	Expr codegen_if(HirNode*);
	Expr codegen_if_chain(HirNode*);
	Expr codegen_if_phi(HirNode*);
	Expr codegen_var(HirNode*);
	Expr codegen_var_member(HirNode*);
	Expr codegen_expr(HirNode*);
	Expr codegen_expr(HirNode* node, Expr lhs);
	Expr codegen_return(HirNode*);
	Expr codegen_number_literal(HirNode*);
	Expr codegen_string_literal(HirNode*);

	llvm::Value* codegen_memcpy(Expr expr, llvm::Value* dest);
	llvm::Value* codegen_eval(Expr expr);
	llvm::Type* get_type(QualifiedTy qty);
	llvm::Type* get_type(Ty const* ty);
	static Codegen codegen(SymBuiltins& builtins, HirNode* module);
};
