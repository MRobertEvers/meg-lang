#pragma once
#include "sema2/Scope.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
class CGTag
{
public:
	llvm::Function* Function;
};

class CGScope
{
public:
	llvm::Function* Function;
};
} // namespace cg