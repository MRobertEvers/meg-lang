#pragma once

#include "./Codegen/LLVMAddress.h"
#include "./Codegen/RValue.h"
#include "./LValue.h"
#include "common/Vec.h"

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

	Vec<CGExpr> discriminations;

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

	void add_discrimination(CGExpr expr);
	CGExpr get_discrimination(int ind);

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