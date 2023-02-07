#pragma once

#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
enum CGExprType
{
	Empty,
	Value,
	FunctionValue,
};

struct CGExpr
{
	bool literal = false;
	enum CGExprType type = CGExprType::Empty;
	union
	{
		llvm::Value* value;
		llvm::Function* fn;
	} data;

	CGExpr(){};
	CGExpr(llvm::Value* value)
		: type(CGExprType::Value)
		, literal(false)
	{
		data.value = value;
	};
	CGExpr(llvm::Value* value, bool literal)
		: type(CGExprType::Value)
		, literal(literal)
	{
		data.value = value;
	};
	CGExpr(llvm::Function* value)
		: type(CGExprType::FunctionValue)
		, literal(false)
	{
		data.fn = value;
	};

	llvm::Value* as_value();
};
} // namespace cg