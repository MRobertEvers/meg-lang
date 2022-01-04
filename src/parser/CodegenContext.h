#pragma once

#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <map>
#include <memory>

class CodegenContext
{
public:
	std::unique_ptr<llvm::LLVMContext> Context;
	std::unique_ptr<llvm::Module> Module;
	std::unique_ptr<llvm::IRBuilder<>> Builder;

	std::map<std::string, std::unique_ptr<llvm::Function>> Functions;

public:
	CodegenContext()
	{
		Context = std::make_unique<llvm::LLVMContext>();
		Module = std::make_unique<llvm::Module>("this_module", *Context);
		// Create a new builder for the module.
		Builder = std::make_unique<llvm::IRBuilder<>>(*Context);
	}
};