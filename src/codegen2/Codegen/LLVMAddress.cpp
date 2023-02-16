#include "LLVMAddress.h"

using namespace cg;

LLVMAddress::LLVMAddress(llvm::Value* ptr, llvm::Type* allocated_type)
	: pointer(ptr)
	, type(allocated_type)
{}

llvm::Type*
LLVMAddress::llvm_allocated_type() const
{
	return this->type;
}

llvm::Value*
LLVMAddress::llvm_pointer() const
{
	return this->pointer;
}
