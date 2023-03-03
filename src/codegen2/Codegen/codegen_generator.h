#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

struct LLVMYieldPoint
{
	llvm::BasicBlock* llvm_suspend_block;
	llvm::BasicBlock* llvm_resume_block;

	CGExpr yield_expr;

	LLVMYieldPoint(llvm::BasicBlock* suspend, llvm::BasicBlock* resume, CGExpr yield_expr)
		: llvm_suspend_block(suspend)
		, llvm_resume_block(resume)
		, yield_expr(yield_expr)
	{}
};

class LLVMAsyncFn
{
public:
	llvm::BasicBlock* entry_block;
	std::vector<LLVMYieldPoint> yield_bbs;
	std::vector<LLVMAddress> allocas;
	llvm::Type* llvm_send_return_type;
	llvm::Type* llvm_frame_type;

	void add_yield(LLVMYieldPoint yield) { yield_bbs.push_back(yield); }
	void add_alloca(LLVMAddress alloca) { allocas.push_back(alloca); }
};

CGResult<CGExpr> codegen_generator(CG&, ir::IRGenerator*);
CGResult<CGExpr> codegen_yield(CG&, cg::LLVMFnInfo&, ir::IRYield*);
// CGResult<LLVMFnSigInfo> codegen_function_proto(CG&, ir::IRProto*);
// CGResult<CGExpr> codegen_function_body(CG&, cg::LLVMFnInfo&, ir::IRBlock*);
} // namespace cg