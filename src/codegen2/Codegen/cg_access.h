#pragma once
#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "sema2/IR.h"

namespace cg
{
class CG;
/**
 * @brief
 *
 * @param src
 * @param type Semantic type of src
 * @param member
 * @return CGResult<CGExpr>
 */
CGResult<CGExpr> cg_access(
	CG&, LLVMAddress src, sema::TypeInstance const& type, sema::MemberTypeInstance const& member);
} // namespace cg