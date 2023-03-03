#include "codegen_return.h"

#include "../Codegen.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_return(CG& codegen, cg::LLVMFnInfo& fn, ir::IRReturn* ir_return)
{
	auto exprr = codegen.codegen_expr(fn, ir_return->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	if( codegen.async_context.has_value() )
	{
		llvm::BasicBlock* llvm_return_block = llvm::BasicBlock::Create(*codegen.Context, "return");
		fn.add_basic_block(llvm_return_block);

		codegen.async_context.value().add_early_return(
			LLVMYieldPoint(codegen.Builder->GetInsertBlock(), llvm_return_block, expr));
		codegen.Builder->SetInsertPoint(llvm_return_block);

		codegen.Builder->CreateRetVoid();
		return CGExpr();
	}

	if( expr.is_empty() )
	{
		codegen.Builder->CreateRetVoid();
		return CGExpr();
	}
	if( fn.has_sret_arg() )
	{
		auto llvm_value = expr.address().llvm_pointer();
		auto sret_arg = fn.sret();
		auto llvm_sret_value = sret_arg.lvalue.address().llvm_pointer();
		auto llvm_sret_type = sret_arg.lvalue.address().llvm_allocated_type();

		// TODO: Compute alignment from member
		auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(llvm_sret_type);
		auto llvm_align = codegen.Module->getDataLayout().getPrefTypeAlign(llvm_sret_type);

		codegen.Builder->CreateMemCpy(
			llvm_sret_value, llvm_align, llvm_value, llvm_align, llvm_size);
		codegen.Builder->CreateRetVoid();
	}
	else
	{
		auto llvm_value = codegen_operand_expr(codegen, expr);
		assert(!llvm_value->getType()->isStructTy());
		codegen.Builder->CreateRet(llvm_value);
	}

	return CGExpr();
}