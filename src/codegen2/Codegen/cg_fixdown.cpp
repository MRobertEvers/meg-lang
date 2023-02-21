

#include "cg_fixdown.h"

#include "../Codegen.h"

using namespace cg;

CGResult<CGExpr>
cg::cg_fixdown(CG& codegen, LLVMAddress const& address, LLVMFixup const& fixup)
{
	auto llvm_dest_value = codegen.Builder->CreateStructGEP(
		address.llvm_allocated_type(), address.llvm_pointer(), fixup.gep());

	auto llvm_value =
		codegen.Builder->CreateBitCast(llvm_dest_value, fixup.llvm_fixdown_type()->getPointerTo());

	auto dest_fixup = LLVMFixup(
		address.llvm_pointer(),
		address.llvm_allocated_type(),
		fixup.gep(),
		fixup.llvm_fixdown_type());

	return CGExpr::MakeAddress(LLVMAddress(llvm_value, fixup.llvm_fixdown_type(), dest_fixup));
}