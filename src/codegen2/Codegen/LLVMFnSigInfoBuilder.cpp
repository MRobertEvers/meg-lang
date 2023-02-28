
#include "LLVMFnSigInfoBuilder.h"

using namespace cg;

LLVMFnSigInfoBuilder::LLVMFnSigInfoBuilder(String name, sema::Type const* sema_fn_ty)
	: name(name)
	, sema_fn_ty(sema_fn_ty)
{}

void
LLVMFnSigInfoBuilder::add_arg_type(cg::LLVMArgABIInfo info)
{
	abi_arg_infos.push_back(info);
	if( info.attr == LLVMArgABIInfo::SRet )
	{
		has_sret_arg_ = info.attr == true;
		sret_arg_ind_ = abi_arg_infos.size() - 1;
		ret_type = LLVMFnSigRetType::SRet;
	}
}

void
LLVMFnSigInfoBuilder::add_arg_type(sema::NameRef name, LLVMArgABIInfo info)
{
	add_arg_type(info);

	named_args_info_inds.emplace(
		name.id().index(),
		std::make_pair<sema::NameRef, int>(sema::NameRef(name), abi_arg_infos.size() - 1));
}

cg::LLVMArgABIInfo
LLVMFnSigInfoBuilder::arg_type(int idx)
{
	assert(idx < abi_arg_infos.size());
	return abi_arg_infos.at(idx);
}

void
LLVMFnSigInfoBuilder::set_llvm_ret_ty(llvm::Type* llvm_ret_ty)
{
	this->llvm_ret_ty = llvm_ret_ty;
}

void
LLVMFnSigInfoBuilder::set_is_var_arg(bool is)
{
	is_var_arg_ = is;
}

bool
LLVMFnSigInfoBuilder::is_var_arg(void) const
{
	return is_var_arg_;
}

Vec<llvm::Type*>
cg::get_llvm_arg_types(Vec<LLVMArgABIInfo> const& abis)
{
	Vec<llvm::Type*> result;

	for( auto& abi : abis )
		result.push_back(abi.llvm_type);

	return result;
}

bool
LLVMFnSigInfoBuilder::has_sret_arg(void) const
{
	return has_sret_arg_;
}

int
LLVMFnSigInfoBuilder::sret_arg_index(void) const
{
	assert(has_sret_arg_);
	return sret_arg_ind_;
}
