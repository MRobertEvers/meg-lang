#include "codegen_alloca.h"

#include "LLVMAddress.h"
#include "lookup.h"
#include <llvm/IR/IRBuilder.h>

using namespace cg;

CGExpr
cg::codegen_decl_var(CG& codegen, ir::Alloca* ir_alloca)
{
	auto llvm_allocated_type = get_type(codegen, ir_alloca->type).value();

	llvm::AllocaInst* llvm_alloca = codegen.Builder->CreateAlloca(llvm_allocated_type, nullptr);

	auto addr = LLVMAddress(llvm_alloca, llvm_allocated_type);

	codegen.vars.emplace(ir_alloca->name_id.index(), addr);

	return CGExpr::MakeAddress(addr);
}