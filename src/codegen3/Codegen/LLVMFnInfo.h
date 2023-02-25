#pragma once

#include "LLVMFnSigInfo.h"
#include "LValue.h"
#include "ir/Type.h"
#include <llvm/IR/IRBuilder.h>

#include <optional>
#include <string>

namespace cg
{

struct LLVMFnArgInfo
{
private:
	std::string name_;
	LLVMFnArgInfo(LLVMArgABIInfo abi, LValue lvalue);
	LLVMFnArgInfo(std::string name, LLVMArgABIInfo abi, LValue lvalue);

public:
	LLVMArgABIInfo abi;
	LValue lvalue;

	bool is_sret() const;
	std::string const& name() const;

	static LLVMFnArgInfo Named(std::string name, LLVMArgABIInfo abi, LValue lvalue);
	static LLVMFnArgInfo SRet(LLVMArgABIInfo abi, LValue lvalue);
};

struct LLVMSwitchInfo
{
	llvm::SwitchInst* switch_inst_;
	LLVMAddress switch_cond_;
	llvm::BasicBlock* default_block_;
	llvm::BasicBlock* after_block_;

	LLVMSwitchInfo(
		llvm::SwitchInst* switch_inst,
		LLVMAddress switch_cond,
		llvm::BasicBlock* after_block,
		llvm::BasicBlock* default_block)
		: switch_inst_(switch_inst)
		, switch_cond_(switch_cond)
		, after_block_(after_block)
		, default_block_(default_block)
	{}

	llvm::SwitchInst* switch_inst() const { return switch_inst_; }
	LLVMAddress switch_cond() const { return switch_cond_; }
	llvm::BasicBlock* merge_bb() const { return after_block_; }
	llvm::BasicBlock* default_bb() const { return default_block_; }
};

/**
 * @brief Contains the llvm argument values and their.
 *
 */
struct LLVMFnInfo
{
	LLVMFnSigInfo sig_info;

	std::map<std::string, LLVMFnArgInfo> named_args;

	std::optional<LLVMFnArgInfo> sret_arg;
	std::optional<llvm::BasicBlock*> merge_block_;
	std::optional<LLVMSwitchInfo> switch_info_;

	LLVMFnInfo(
		LLVMFnSigInfo sig_info,
		std::map<std::string, LLVMFnArgInfo> named_args,
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

	// TODO: See above
	// How to keep track that we're in a switch?
	std::optional<LLVMSwitchInfo> switch_inst() { return switch_info_; }
	void set_switch_inst(std::optional<LLVMSwitchInfo> bl) { switch_info_ = bl; }
	void clear_switch_inst() { switch_info_.reset(); }
};
} // namespace cg
