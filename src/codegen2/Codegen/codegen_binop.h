#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{

class CG;

struct SemaTypedInt
{
	sema::TypeInstance type;
	llvm::Value* value;

	SemaTypedInt(sema::TypeInstance type, llvm::Value* value)
		: type(type)
		, value(value)
	{}
};

CGResult<CGExpr> codegen_arithmetic_binop(CG&, SemaTypedInt, SemaTypedInt, ast::BinOp);
CGResult<CGExpr> codegen_binop(CG&, cg::LLVMFnInfo&, ir::IRBinOp*);
} // namespace cg