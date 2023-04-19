#pragma once

#include <llvm/IR/IRBuilder.h>

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