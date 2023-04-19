#pragma once

#include <llvm/IR/IRBuilder.h>

class Function
{
	llvm::Function* llvm_func;

public:
	Function(llvm::Function* llvm_func);
};