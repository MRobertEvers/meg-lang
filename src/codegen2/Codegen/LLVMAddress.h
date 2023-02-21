#pragma once

#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <optional>

namespace cg
{

// For addresses the point to internal members of enums (future: arrays)
// The fixup contains info on how to fixup and fixdown.
// This is only for single geps.
class LLVMFixup
{
	// This is the containing type.
	llvm::Value* pointer;
	llvm::Type* type;

	// This is the type internal to the contained type
	int gep_;
	llvm::Type* fixdown_type;

public:
	LLVMFixup() = default;
	LLVMFixup(llvm::Value* ptr, llvm::Type* allocated_type, int gep, llvm::Type* fixdown);

	llvm::Type* llvm_fixdown_type() const;
	int gep() const;
	llvm::Type* llvm_allocated_type() const;
	llvm::Value* llvm_pointer() const;
};

class LLVMAddress
{
	llvm::Value* pointer;
	llvm::Type* type;

	// For enums and arrays.
	// The address might be pointing to an element,
	// of the enum.
	// This is a pointer to the containing enum.
	std::optional<LLVMFixup> fixup_;

public:
	LLVMAddress(llvm::Value* ptr, llvm::Type* allocated_type);
	LLVMAddress(llvm::Value*, llvm::Type*, LLVMFixup);

	llvm::Type* llvm_allocated_type() const;
	llvm::Value* llvm_pointer() const;

	// TODO: Better name
	LLVMAddress fixup() const;
	std::optional<LLVMFixup> fixup_info() const;
};
} // namespace cg