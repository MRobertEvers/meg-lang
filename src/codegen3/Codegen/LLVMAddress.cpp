#include "LLVMAddress.h"

using namespace cg;

LLVMFixup::LLVMFixup(llvm::Value* ptr, llvm::Type* allocated_type, int gep, llvm::Type* fixdown)
	: pointer(ptr)
	, type(allocated_type)
	, fixdown_type(fixdown)
	, gep_(gep)
{}

llvm::Type*
LLVMFixup::llvm_fixdown_type() const
{
	return this->fixdown_type;
}

int
LLVMFixup::gep() const
{
	return this->gep_;
}

llvm::Type*
LLVMFixup::llvm_allocated_type() const
{
	return this->type;
}

llvm::Value*
LLVMFixup::llvm_pointer() const
{
	return this->pointer;
}

LLVMAddress::LLVMAddress(llvm::Value* ptr, llvm::Type* allocated_type)
	: pointer(ptr)
	, type(allocated_type)
{}

LLVMAddress::LLVMAddress(llvm::Value* ptr, llvm::Type* allocated_type, LLVMFixup fixup)
	: pointer(ptr)
	, type(allocated_type)
	, fixup_(fixup)
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

LLVMAddress
LLVMAddress::fixup() const
{
	if( this->fixup_.has_value() )
		return LLVMAddress(
			this->fixup_.value().llvm_pointer(), this->fixup_.value().llvm_allocated_type());
	else
		return *this;
}

std::optional<LLVMFixup>
LLVMAddress::fixup_info() const
{
	return this->fixup_;
}