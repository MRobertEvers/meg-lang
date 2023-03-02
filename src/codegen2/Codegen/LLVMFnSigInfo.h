#pragma once

#include "../Scope.h"
#include "LLVMArgABIInfo.h"
#include "common/String.h"
#include "common/Vec.h"
#include "sema2/Name.h"
#include "sema2/type/Type.h"
#include <llvm/IR/IRBuilder.h>

#include <map>
#include <optional>
namespace cg
{

enum class LLVMFnSigRetType
{
	SRet,
	Default
};

struct LLVMFnSigInfo
{
private:
	bool is_var_arg_;
	int sret_arg_ind_;

	// NameId -> Index in abi_arg_infos
	std::map<int, std::pair<sema::NameRef, int>> named_args_info_inds_;

public:
	// These may be empty.
	sema::NameRef name;
	sema::Type const* sema_fn_ty;

	llvm::Function* llvm_fn;
	llvm::Type* llvm_fn_ty;
	Vec<LLVMArgABIInfo> abi_arg_infos;
	LLVMFnSigRetType ret_type = LLVMFnSigRetType::Default;

	LLVMFnSigInfo(
		llvm::Function*,
		llvm::Type*,
		Vec<LLVMArgABIInfo>,
		std::map<int, std::pair<sema::NameRef, int>>,
		sema::Type const*,
		LLVMFnSigRetType,
		int,
		bool);

	LLVMFnSigInfo(
		sema::NameRef,
		llvm::Function*,
		llvm::Type*,
		Vec<LLVMArgABIInfo>,
		std::map<int, std::pair<sema::NameRef, int>>,
		sema::Type const*,
		LLVMFnSigRetType,
		int,
		bool);

	std::optional<sema::NameRef> get_arg_name(int idx);

	LLVMArgABIInfo arg_type(int idx) const;
	int nonvar_arg_count(void) const;
	bool is_var_arg(void) const;
	bool has_sret_arg(void) const;
	int sret_arg_index(void) const;
	bool is_void_rt(void) const;
};

} // namespace cg
