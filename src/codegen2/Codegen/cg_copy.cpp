#include "cg_copy.h"

#include "../Codegen.h"
#include "cg_fixdown.h"

using namespace cg;

CGResult<CGExpr>
cg::cg_copy_alloca(CG& codegen, LLVMAddress& src)
{
	//
	auto src_address = src.fixup();

	llvm::AllocaInst* llvm_cpy_alloca = codegen.builder_alloca(src_address.llvm_allocated_type());
	auto dest_address = LLVMAddress(llvm_cpy_alloca, src_address.llvm_allocated_type());

	return cg_copy(codegen, src_address, dest_address);
}

CGExpr
cg::cg_copy(CG& codegen, LLVMAddress& src, LLVMAddress& dest)
{
	auto src_address = src.fixup();

	auto llvm_size =
		codegen.Module->getDataLayout().getTypeAllocSize(src_address.llvm_allocated_type());
	auto llvm_align =
		codegen.Module->getDataLayout().getPrefTypeAlign(src_address.llvm_allocated_type());

	codegen.Builder->CreateMemCpy(
		dest.llvm_pointer(), llvm_align, src_address.llvm_pointer(), llvm_align, llvm_size);

	auto maybe_fixup_info = src.fixup_info();
	if( maybe_fixup_info.has_value() )
		return cg_fixdown(codegen, dest, maybe_fixup_info.value()).unwrap();
	else
		return CGExpr::MakeAddress(dest);
}