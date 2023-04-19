

#pragma once

#include <llvm/IR/IRBuilder.h>

class Address
{
	llvm::Value* pointer;
	llvm::Type* type;

public:
	Address(llvm::Value* ptr, llvm::Type* allocated_type);

	llvm::Type* llvm_allocated_type() const;
	llvm::Value* llvm_pointer() const;
};