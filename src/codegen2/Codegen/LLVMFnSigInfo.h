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
 * @brief Pass by value still uses pointers.
 * The difference is that by-value structs are copied to their own
 * memory location before the call and a pointer to that is passed in.
 *
 */
struct LLVMArgABIInfo
{
	enum Kind : char
	{
		Default,
		SRet,
		Value,
	};

	Kind attr = Default;
	llvm::Type* type;

	LLVMArgABIInfo(Kind attr, llvm::Type* type)
		: attr(attr)
		, type(type){};
};

/**
 * @brief Captures ABI information about arguments to an LLVM function.
 *
 */
struct LLVMFnSigInfo
{
	enum class RetType
	{
		SRet,
		Default
	};

	llvm::Function* Fn;
	llvm::Type* FnType;
	Vec<LLVMArgABIInfo> ArgsTypes;
	bool is_var_arg;

	sema::Type const* fn_type;
	RetType ret_type = RetType::Default;

	Vec<Scope> scopes;
	Scope* current_scope;

	LLVMFnSigInfo(
		llvm::Function* Fn,
		llvm::Type* Type,
		Vec<LLVMArgABIInfo> ArgsTypes,
		bool is_var_arg,
		sema::Type const* fn_type,
		RetType ret_type);

	void add_arg_type(LLVMArgABIInfo);
	LLVMArgABIInfo arg_type(int idx);
	void add_lvalue(String const& name, LValue lvalue);
};
} // namespace cg
