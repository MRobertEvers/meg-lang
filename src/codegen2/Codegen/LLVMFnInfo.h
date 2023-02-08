#pragma once

#include "../Scope.h"
#include "common/Vec.h"
#include "sema2/type/Type.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{

/**
 * @brief Contains the llvm argument values and their.
 *
 */
struct LLVMFnInfo
{
	enum class RetType
	{
		SRet,
		Default
	};

	llvm::Function* Fn;
	llvm::Type* FnType;
	Vec<ArgumentType> ArgsTypes;
	bool is_var_arg;

	sema::Type const* fn_type;
	RetType ret_type = RetType::Default;

	Vec<Scope> scopes;
	Scope* current_scope;

	LLVMFnInfo(
		llvm::Function* Fn,
		llvm::Type* Type,
		Vec<ArgumentType> ArgsTypes,
		bool is_var_arg,
		sema::Type const* fn_type,
		RetType ret_type);

	void add_arg_type(ArgumentType);
	ArgumentType arg_type(int idx);
	void add_lvalue(String const& name, LValue lvalue);
};
} // namespace cg
