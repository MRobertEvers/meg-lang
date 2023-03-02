#include "LLVMFnSigInfo.h"

using namespace cg;

LLVMFnSigInfo::LLVMFnSigInfo(
	llvm::Function* llvm_fn,
	llvm::Type* llvm_fn_ty,
	Vec<LLVMArgABIInfo> abi_infos,
	std::map<int, std::pair<sema::NameRef, int>> named_args,
	sema::Type const* sema_ty,
	LLVMFnSigRetType ret_type,
	int sret_arg_index,
	bool var_args)
	: llvm_fn(llvm_fn)
	, llvm_fn_ty(llvm_fn_ty)
	, abi_arg_infos(abi_infos)
	, named_args_info_inds_(named_args)
	, sema_fn_ty(sema_ty)
	, ret_type(ret_type)
	, sret_arg_ind_(sret_arg_index)
	, is_var_arg_(var_args)
{}

LLVMFnSigInfo::LLVMFnSigInfo(
	sema::NameRef name,
	llvm::Function* llvm_fn,
	llvm::Type* llvm_fn_ty,
	Vec<LLVMArgABIInfo> abi_infos,
	std::map<int, std::pair<sema::NameRef, int>> named_args,
	sema::Type const* sema_ty,
	LLVMFnSigRetType ret_type,
	int sret_arg_index,
	bool var_args)
	: name(name)
	, llvm_fn(llvm_fn)
	, llvm_fn_ty(llvm_fn_ty)
	, abi_arg_infos(abi_infos)
	, named_args_info_inds_(named_args)
	, sema_fn_ty(sema_ty)
	, ret_type(ret_type)
	, sret_arg_ind_(sret_arg_index)
	, is_var_arg_(var_args)
{}

std::optional<sema::NameRef>
LLVMFnSigInfo::get_arg_name(int idx)
{
	for( auto& [name, ind] : named_args_info_inds_ )
	{
		if( idx == ind.second )
			return ind.first;
	}

	return std::optional<sema::NameRef>();
}

LLVMArgABIInfo
LLVMFnSigInfo::arg_type(int idx) const
{
	if( idx < abi_arg_infos.size() )
		return abi_arg_infos.at(idx);
	else if( is_var_arg_ )
		return LLVMArgABIInfo::Unchecked();
	else
	{
		assert(
			0 && "Attempting to find abi info for argument that is out of bounds of a non-vararg "
				 "fn.");
		return abi_arg_infos.at(idx);
	}
}

int
LLVMFnSigInfo::nonvar_arg_count(void) const
{
	return abi_arg_infos.size();
}

bool
LLVMFnSigInfo::is_var_arg(void) const
{
	return is_var_arg_;
}

bool
LLVMFnSigInfo::has_sret_arg(void) const
{
	return ret_type == LLVMFnSigRetType::SRet;
}

int
LLVMFnSigInfo::sret_arg_index(void) const
{
	assert(has_sret_arg());
	return sret_arg_ind_;
}

bool
LLVMFnSigInfo::is_void_rt(void) const
{
	return true;
}