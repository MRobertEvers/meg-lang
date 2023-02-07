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
	llvm::Function* Fn;
	Vec<llvm::Value*> Args;

	sema::Type const* fn_type;
	CGFunctionContext(llvm::Function* Fn, sema::Type const* fn_type)
		: Fn(Fn)
		, fn_type(fn_type){};
};
} // namespace cg
