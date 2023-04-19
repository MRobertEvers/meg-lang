#include "Address.h"

Address::Address(llvm::Value* ptr, llvm::Type* ty)
	: pointer(ptr)
	, type(ty)
{}

llvm::Type*
Address::llvm_allocated_type() const
{
	return this->type;
}

llvm::Value*
Address::llvm_pointer() const
{
	return this->pointer;
}
