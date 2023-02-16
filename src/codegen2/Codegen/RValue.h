#pragma once

#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
class RValue
{
	llvm::Value* pointer;
	llvm::Type* type;

public:
	RValue(llvm::Value* ptr, llvm::Type* type);
	RValue(llvm::Value* ptr);

	llvm::Type* llvm_type() const;
	llvm::Value* llvm_pointer() const;
};
} // namespace cg