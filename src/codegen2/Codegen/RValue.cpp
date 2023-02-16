
#include "RValue.h"

using namespace cg;

RValue::RValue(llvm::Value* ptr, llvm::Type* ty)
	: pointer(ptr)
	, type(ty)
{}

RValue::RValue(llvm::Value* ptr)
	: pointer(ptr)
	, type(ptr->getType())
{}

llvm::Type*
RValue::llvm_type() const
{
	return this->type;
}

llvm::Value*
RValue::llvm_pointer() const
{
	return this->pointer;
}
