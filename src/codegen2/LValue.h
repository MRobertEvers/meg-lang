#pragma once

#include <llvm/IR/Value.h>

namespace cg
{

class LValue
{
	llvm::Value* V;
	llvm::Type* ElementType;

public:
	LValue(llvm::Value* V, llvm::Type* ElementType)
		: V(V)
		, ElementType(ElementType){};

	llvm::Value* value() const { return V; }
	llvm::Type* type() const { return ElementType; }
};

} // namespace cg