#pragma once

#include "Address.h"
#include "Expr.h"
#include "sema3/Sym.h"
#include <llvm/IR/IRBuilder.h>

#include <vector>

/**
 * By default, args are passed and then alloca'd.
 *
 * For SRet and ByVal params, these are not alloca'd.
 */
class Arg
{
private:
	bool is_sret_;
	bool is_byval_;

public:
	Sym* sym = nullptr;
	llvm::Type* type = nullptr;
	Arg(Sym* sym, llvm::Type* type, bool is_sret_, bool is_byval_);

	bool is_sret() const;
	bool is_byval() const;
};

struct FunctionYieldPoint
{
	llvm::BasicBlock* suspend_bb;
	llvm::BasicBlock* resume_bb;
	Expr yield_expr;

	FunctionYieldPoint(llvm::BasicBlock* suspend_bb, llvm::BasicBlock* resume_bb, Expr yield_expr);
};

class Function
{
public:
	Expr sret;
	std::vector<int> ir_args;
	std::vector<Arg> args;
	std::vector<Address> allocas;
	std::vector<FunctionYieldPoint> yield_points;

	// For async functions only.
	llvm::Type* llvm_send_opt_ty = nullptr;
	llvm::Type* llvm_send_ty = nullptr;

	llvm::Function* llvm_func;
	Function(llvm::Function* llvm_func, std::vector<Arg> args, Expr sret);

	// Corresponds to the argument in the ith position in the intermediate representation.
	// TODO: This doesn't actually correspond anymore because of async functions, all arguments
	// to async functions are non-ir
	Arg& ir_arg(int i);
	int ir_arg_count() const;
	int llvm_arg_index(int ir_index);

	static Function FromArgs(llvm::Function* llvm_func, std::vector<Arg> args);
};