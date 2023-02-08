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

enum class ArgumentAttr
{
	Default,
	SRet,
	Value,
};

/**
 * @brief Pass by value still uses pointers.
 * The difference is that by-value structs are copied to their own
 * memory location before the call and a pointer to that is passed in.
 *
 */
struct ArgumentType
{
	ArgumentAttr attr = ArgumentAttr::Default;
	llvm::Type* type;

	ArgumentType(ArgumentAttr attr, llvm::Type* type)
		: attr(attr)
		, type(type){};
};

struct CGFunctionContext
{
	enum class RetType
	{
		SRet,
		Default
	};

	llvm::Function* Fn;
	llvm::Type* FnType;
	Vec<ArgumentType> ArgsTypes;

	union
	{
		LValue sret;
	};

	sema::Type const* fn_type;
	RetType ret_type = RetType::Default;

	Vec<Scope> scopes;
	Scope* current_scope;

	CGFunctionContext(
		llvm::Function* Fn,
		llvm::Type* Type,
		Vec<ArgumentType> ArgsTypes,
		sema::Type const* fn_type,
		RetType ret_type);

	void add_arg_type(ArgumentType);
	ArgumentType arg_type(int idx);
	void add_lvalue(String const& name, LValue lvalue);
};
} // namespace cg
