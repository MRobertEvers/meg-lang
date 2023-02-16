#pragma once

#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
class LLVMAddress
{
	llvm::Value* pointer;
	llvm::Type* type;

public:
	LLVMAddress(llvm::Value* ptr, llvm::Type* allocated_type);

	llvm::Type* llvm_allocated_type() const;
	llvm::Value* llvm_pointer() const;
};
} // namespace cg