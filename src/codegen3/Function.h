#pragma once

#include <llvm/IR/IRBuilder.h>

class Function
{
public:
	llvm::Function* llvm_func;
	Function(llvm::Function* llvm_func);
};