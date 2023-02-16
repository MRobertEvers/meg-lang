
#include "codegen_while.h"

#include "../Codegen.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_while(CG& codegen, cg::LLVMFnInfo& fn, ir::IRWhile* ir_while)
{
	auto llvm_fn = fn.llvm_fn();
	llvm::BasicBlock* llvm_cond_bb =
		llvm::BasicBlock::Create(*codegen.Context, "condition", llvm_fn);
	llvm::BasicBlock* llvm_loop_bb = llvm::BasicBlock::Create(*codegen.Context, "loop");
	llvm::BasicBlock* llvm_done_bb = llvm::BasicBlock::Create(*codegen.Context, "after_loop");

	// Insert an explicit fall through from the current block to the LoopBB.
	// TODO: (2022-01-11) I'm not sure why this is needed, but LLVM segfaults if its not there.
	// Maybe it is doing some sort of graph analysis and implicit fallthrough on blocks
	// is not allowed?
	// Answer: (2022-05-25) Yes. It is. Every block must end with a terminator instruction, the
	// block we are in when we enter this function (i.e. visit(ast:While)) does not yet have a
	// terminator instr.
	codegen.Builder->CreateBr(llvm_cond_bb);
	codegen.Builder->SetInsertPoint(llvm_cond_bb);

	auto condr = codegen.codegen_expr(fn, ir_while->condition);
	if( !condr.ok() )
		return condr;
	auto cond = condr.unwrap();

	auto llvm_cond = codegen_operand_expr(codegen, cond);
	// TODO: Typecheck is boolean or cast?
	codegen.Builder->CreateCondBr(llvm_cond, llvm_loop_bb, llvm_done_bb);
	llvm_fn->getBasicBlockList().push_back(llvm_loop_bb);
	codegen.Builder->SetInsertPoint(llvm_loop_bb);

	auto bodyr = codegen.codegen_stmt(fn, ir_while->body);
	if( !bodyr.ok() )
		return bodyr;
	auto body = bodyr.unwrap();

	codegen.Builder->CreateBr(llvm_cond_bb);

	llvm_fn->getBasicBlockList().push_back(llvm_done_bb);
	codegen.Builder->SetInsertPoint(llvm_done_bb);

	return CGExpr();
}