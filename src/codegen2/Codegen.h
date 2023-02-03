#pragma once
#include "CGTag.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
class CG
{
public:
	using TagType = CGTag;
};
} // namespace cg