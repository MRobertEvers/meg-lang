#pragma once

#include "common/Vec.h"
#include "sema2/type/Type.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{

struct CGFunctionContext
{
	enum class RetType
	{
		SRet,
		Default
	};

	llvm::Function* Fn;
	llvm::Type* FnType;

	Vec<llvm::Value*> Args;
	sema::Type const* fn_type;
	RetType ret_type = RetType::Default;

	CGFunctionContext(
		llvm::Function* Fn, llvm::Type* Type, sema::Type const* fn_type, RetType ret_type)
		: Fn(Fn)
		, FnType(Type)
		, fn_type(fn_type)
		, ret_type(ret_type){};
};
} // namespace cg
