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
	std::vector<LLVMYieldPoint> early_return_blocks;
	std::vector<LLVMYieldPoint> yield_bbs;
	std::vector<LLVMAddress> allocas;
	llvm::Type* llvm_send_return_type;
	llvm::Type* llvm_send_type;
	// The argument to the send function is an Optional<T>
	// this is that type.
	llvm::Type* llvm_send_optional_type;
	llvm::Type* llvm_frame_type;
	llvm::Type* llvm_ret_type;

	int return_val_idx;

	sema::TypeInstance sema_send_type;
	sema::TypeInstance sema_yield_type;
	sema::TypeInstance sema_return_type;

	void add_early_return(LLVMYieldPoint early_ret) { early_return_blocks.push_back(early_ret); }
	void add_yield(LLVMYieldPoint yield) { yield_bbs.push_back(yield); }
	void add_alloca(LLVMAddress alloca) { allocas.push_back(alloca); }
};

CGResult<CGExpr> codegen_generator(CG&, ir::IRGenerator*);
CGResult<CGExpr> codegen_yield(CG&, cg::LLVMFnInfo&, ir::IRYield*);
// CGResult<LLVMFnSigInfo> codegen_function_proto(CG&, ir::IRProto*);
// CGResult<CGExpr> codegen_function_body(CG&, cg::LLVMFnInfo&, ir::IRBlock*);
} // namespace cg