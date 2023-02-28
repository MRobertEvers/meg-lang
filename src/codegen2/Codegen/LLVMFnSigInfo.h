#pragma once

#include "../Scope.h"
#include "common/String.h"
#include "common/Vec.h"
#include "sema2/Name.h"
#include "sema2/type/Type.h"
#include <llvm/IR/IRBuilder.h>

#include <map>
#include <optional>
namespace cg
{

/**
 * @brief Pass by value still uses pointers.
 * The difference is that by-value structs are copied to their own
 * memory location before the call and a pointer to that is passed in.
 *
 */
struct LLVMArgABIInfo
{
	enum Kind : char
	{
		Default,
		SRet,
		Value,

		// Indicates that this ABI Arg Info contains
		// no information about the arg becuase it was
		// provided as a var arg. User must check
		// the type of the argument by examining
		// the expression type.
		UncheckedVarArg
	};

	Kind attr = Default;
	llvm::Type* llvm_type;

	LLVMArgABIInfo(Kind attr, llvm::Type* llvm_type)
		: attr(attr)
		, llvm_type(llvm_type){};

	bool is_sret() const { return attr == SRet; }

	static LLVMArgABIInfo Unchecked()
	{
		return LLVMArgABIInfo(LLVMArgABIInfo::UncheckedVarArg, nullptr);
	}
};

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

	// NameId -> Index
	std::map<int, std::pair<sema::NameRef, int>> named_args_info_inds_;

public:
	String name;
	sema::Type const* sema_fn_ty;

	llvm::Function* llvm_fn;
	llvm::Type* llvm_fn_ty;
	Vec<LLVMArgABIInfo> abi_arg_infos;
	LLVMFnSigRetType ret_type = LLVMFnSigRetType::Default;

	LLVMFnSigInfo(
		String,
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
