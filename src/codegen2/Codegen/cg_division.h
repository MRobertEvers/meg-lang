#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "sema2/TypeInstance.h"

namespace cg
{
class CG;
llvm::Value* cg_division(
	CG&, llvm::Value* numerator, llvm::Value* denominator, sema::TypeInstance numerator_type);
} // namespace cg