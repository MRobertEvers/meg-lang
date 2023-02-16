#pragma once

#include "./Codegen/LLVMAddress.h"
#include "./Codegen/RValue.h"
#include "./LValue.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

namespace cg
{
enum class CGExprType
{
	Empty,
	Address,
	RValue,
};

class CGExpr
{
	CGExprType type = CGExprType::Empty;
	union
	{
		RValue rvalue_;
		LLVMAddress address_;
	};

	CGExpr(LLVMAddress value)
		: type(CGExprType::Address)
		, address_(value){};
	CGExpr(RValue value)
		: type(CGExprType::RValue)
		, rvalue_(value){};

public:
	CGExpr(){};

	static CGExpr MakeAddress(LValue addr);
	static CGExpr MakeAddress(LLVMAddress addr);
	static CGExpr MakeRValue(RValue addr);

	bool is_empty() const { return type == CGExprType::Empty; }
	bool is_rvalue() const { return type == CGExprType::RValue; }
	bool is_address() const { return type == CGExprType::Address; }

	LLVMAddress address() const
	{
		assert(is_address());
		return address_;
	}

	RValue rvalue() const
	{
		assert(is_rvalue());
		return rvalue_;
	}
};
} // namespace cg