#pragma once

#include "../Codegen.h"
#include "LLVMFnSigInfo.h"
#include "common/String.h"
#include "common/Vec.h"

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
	Vec<LLVMArgABIInfo> abi_arg_infos;
	std::map<String, int> named_args_info_inds;

	sema::Type const* sema_fn_ty;
	String name;
	llvm::Type* llvm_ret_ty;
	LLVMFnSigRetType ret_type = LLVMFnSigRetType::Default;

	LLVMFnSigInfoBuilder(String name, sema::Type const* sema_ty);

	void add_arg_type(LLVMArgABIInfo);
	void add_arg_type(String, LLVMArgABIInfo);
	LLVMArgABIInfo arg_type(int idx);

	void set_llvm_ret_ty(llvm::Type*);

	void set_is_var_arg(bool);
	bool is_var_arg(void) const;
	bool has_sret_arg(void) const;
	int sret_arg_index(void) const;
};

Vec<llvm::Type*> get_llvm_arg_types(Vec<LLVMArgABIInfo> const& abis);
} // namespace cg