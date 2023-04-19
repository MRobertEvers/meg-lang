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

class Function
{
public:
	Expr sret;
	std::vector<Arg> args;
	std::vector<Address> allocas;

	llvm::Function* llvm_func;
	Function(llvm::Function* llvm_func, std::vector<Arg> args, Expr sret);

	// Corresponds to the argument in the ith position in the intermediate representation.
	Arg& ir_arg(int i);
	int ir_arg_count() const;
	int llvm_arg_index(int ir_index);

	static Function FromArgs(llvm::Function* llvm_func, std::vector<Arg> args);
};