#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "LLVMAddress.h"
#include "LLVMFnInfo.h"
#include "sema2/IR.h"

namespace cg
{
class CG;

class LLVMAsyncFn
{
public:
	std::vector<llvm::BasicBlock*> yield_bbs;
	std::vector<LLVMAddress> allocas;
	llvm::Type* llvm_send_return_type;
	llvm::Type* llvm_frame_type;

	void add_yield_bb(llvm::BasicBlock* bb) { yield_bbs.push_back(bb); }
	void add_alloca(LLVMAddress alloca) { allocas.push_back(alloca); }
};

CGResult<CGExpr> codegen_generator(CG&, ir::IRGenerator*);
CGResult<CGExpr> codegen_yield(CG&, cg::LLVMFnInfo&, ir::IRYield*);
// CGResult<LLVMFnSigInfo> codegen_function_proto(CG&, ir::IRProto*);
// CGResult<CGExpr> codegen_function_body(CG&, cg::LLVMFnInfo&, ir::IRBlock*);
} // namespace cg