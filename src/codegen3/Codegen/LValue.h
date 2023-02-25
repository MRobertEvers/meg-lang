#pragma once

#include "LLVMAddress.h"
#include <llvm/IR/Value.h>

namespace cg
{

class LValue
{
	LLVMAddress address_;

public:
	LValue(LValue const&) = default;
	LValue(llvm::Value* ptr, llvm::Type* allocated_type)
		: address_(LLVMAddress(ptr, allocated_type))
	{}
	LValue(LLVMAddress address)
		: address_(address)
	{}

	LLVMAddress address() const { return address_; }
};

} // namespace cg