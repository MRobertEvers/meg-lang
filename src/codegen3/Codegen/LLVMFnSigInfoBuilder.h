#pragma once

#include "../Codegen.h"
#include "LLVMFnSigInfo.h"

#include <string>
#include <vector>

namespace cg
{

/**
 * @brief Captures ABI information about arguments to an LLVM function.
 *
 */
struct LLVMFnSigInfoBuilder
{
private:
	bool is_var_arg_;
	bool has_sret_arg_;
	int sret_arg_ind_;

public:
	std::vector<LLVMArgABIInfo> abi_arg_infos;
	std::map<std::string, int> named_args_info_inds;

	ir::Type const* sema_fn_ty;
	std::string name;
	llvm::Type* llvm_ret_ty;
	LLVMFnSigRetType ret_type = LLVMFnSigRetType::Default;

	LLVMFnSigInfoBuilder(std::string name, ir::Type const* sema_ty);

	void add_arg_type(LLVMArgABIInfo);
	void add_arg_type(std::string, LLVMArgABIInfo);
	LLVMArgABIInfo arg_type(int idx);

	void set_llvm_ret_ty(llvm::Type*);

	void set_is_var_arg(bool);
	bool is_var_arg(void) const;
	bool has_sret_arg(void) const;
	int sret_arg_index(void) const;
};

std::vector<llvm::Type*> get_llvm_arg_types(std::vector<LLVMArgABIInfo> const& abis);
} // namespace cg