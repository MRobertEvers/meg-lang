#include "cg_enum_helpers.h"

#include "../Codegen.h"

using namespace cg;

LLVMAddress
cg::cg_enum_nominal(CG& codegen, LLVMAddress const& address)
{
	// TODO: Handle fixups
	auto llvm_enum_nominal_value =
		codegen.Builder->CreateStructGEP(address.llvm_allocated_type(), address.llvm_pointer(), 0);

	llvm::Type* llvm_int_type = llvm::Type::getInt32Ty(*codegen.Context);
	// TODO: Enum backing type
	return LLVMAddress(llvm_enum_nominal_value, llvm_int_type);
}