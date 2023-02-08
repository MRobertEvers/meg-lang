#include "LLVMFnInfoBuilder.h"

using namespace cg;

LLVMFnInfoBuilder::LLVMFnInfoBuilder(LLVMFnSigInfo sig_info)
	: sig_info(sig_info)
{}

void
LLVMFnInfoBuilder::add_arg(LLVMFnArgInfo arg_info)
{
	if( arg_info.is_sret() )
	{
		this->sret_arg = arg_info;
	}
	else
	{
		this->args.emplace(arg_info.name(), arg_info);
	}
}

LLVMFnInfo
LLVMFnInfoBuilder::to_fn_info() const
{
	return LLVMFnInfo(sig_info, args, sret_arg);
}