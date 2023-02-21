#include "cg_copy.h"

#include "../Codegen.h"
#include "cg_fixdown.h"

using namespace cg;

CGResult<CGExpr>
cg::cg_copy(CG& codegen, LLVMAddress& src)
{
	//
	auto src_address = src.fixup();

	llvm::AllocaInst* llvm_cpy_alloca =
		codegen.Builder->CreateAlloca(src_address.llvm_allocated_type(), nullptr);
	auto llvm_size =
		codegen.Module->getDataLayout().getTypeAllocSize(src_address.llvm_allocated_type());
	auto llvm_align =
		codegen.Module->getDataLayout().getPrefTypeAlign(src_address.llvm_allocated_type());

	codegen.Builder->CreateMemCpy(
		llvm_cpy_alloca, llvm_align, src_address.llvm_pointer(), llvm_align, llvm_size);

	auto dest_address = LLVMAddress(llvm_cpy_alloca, src_address.llvm_allocated_type());

	auto maybe_fixup_info = src.fixup_info();
	if( maybe_fixup_info.has_value() )
		return cg_fixdown(codegen, dest_address, maybe_fixup_info.value());
	else
		return CGExpr::MakeAddress(dest_address);
}