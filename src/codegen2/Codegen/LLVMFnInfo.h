#pragma once

#include "../LValue.h"
#include "../Scope.h"
#include "LLVMFnSigInfo.h"
#include "common/Vec.h"
#include "sema2/type/Type.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <optional>

namespace cg
{

struct LLVMFnArgInfo
{
private:
	String name_;
	LLVMFnArgInfo(LLVMArgABIInfo abi, LValue lvalue);
	LLVMFnArgInfo(String name, LLVMArgABIInfo abi, LValue lvalue);

public:
	LLVMArgABIInfo abi;
	LValue lvalue;

	bool is_sret() const;
	String const& name() const;

	static LLVMFnArgInfo Named(String name, LLVMArgABIInfo abi, LValue lvalue);
	static LLVMFnArgInfo SRet(LLVMArgABIInfo abi, LValue lvalue);
};

/**
 * @brief Contains the llvm argument values and their.
 *
 */
struct LLVMFnInfo
{
	LLVMFnSigInfo sig_info;

	std::map<String, LLVMFnArgInfo> named_args;

	std::optional<LLVMFnArgInfo> sret_arg;
	std::optional<llvm::BasicBlock*> merge_block_;

	LLVMFnInfo(
		LLVMFnSigInfo sig_info,
		std::map<String, LLVMFnArgInfo> named_args,
		std::optional<LLVMFnArgInfo> sret_arg);

	bool has_sret_arg() const;
	LLVMFnArgInfo sret() const;

	llvm::Function* llvm_fn() const { return sig_info.llvm_fn; }

	// TODO: This is inviting bad code.
	// How to keep track of if - else if - chains, without
	// creating a huge chain of merge blocks.
	// Is this the best place to communicate this information.
	std::optional<llvm::BasicBlock*> merge_block() { return merge_block_; }
	void set_merge_block(std::optional<llvm::BasicBlock*> bl) { merge_block_ = bl; }
	void clear_merge_block() { merge_block_.reset(); }
};
} // namespace cg
