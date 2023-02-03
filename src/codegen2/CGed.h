#pragma once

#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
enum CGedType
{
	Empty,
	Value
};

struct CGed
{
	enum CGedType type = CGedType::Empty;
	union
	{
		llvm::Value* value;
	} data;

	CGed(){};
	CGed(llvm::Value* value)
		: type(CGedType::Value)
	{
		data.value = value;
	};
};
} // namespace cg