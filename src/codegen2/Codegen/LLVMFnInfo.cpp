#include "LLVMFnInfo.h"

using namespace cg;

LLVMFnArgInfo::LLVMFnArgInfo(LLVMArgABIInfo abi, LValue lvalue)
	: abi(abi)
	, lvalue(lvalue)
{}

LLVMFnArgInfo::LLVMFnArgInfo(sema::NameRef name, LLVMArgABIInfo abi, LValue lvalue)
	: name_(name)
	, abi(abi)
	, lvalue(lvalue)
{}

bool
LLVMFnArgInfo::is_sret() const
{
	return abi.attr == LLVMArgABIInfo::SRet;
}

sema::NameRef
LLVMFnArgInfo::name() const
{
	return name_.value();
}

LLVMFnArgInfo
LLVMFnArgInfo::Named(sema::NameRef name, LLVMArgABIInfo abi, LValue lvalue)
{
	return LLVMFnArgInfo(name, abi, lvalue);
}

LLVMFnArgInfo
LLVMFnArgInfo::SRet(LLVMArgABIInfo abi, LValue lvalue)
{
	return LLVMFnArgInfo(abi, lvalue);
}

LLVMFnInfo::LLVMFnInfo(
	LLVMFnSigInfo sig_info,
	std::map<int, LLVMFnArgInfo> named_args,
	std::optional<LLVMFnArgInfo> sret_arg)
	: named_args(named_args)
	, sig_info(sig_info)
	, sret_arg(sret_arg)
{}

bool
LLVMFnInfo::has_sret_arg() const
{
	return sret_arg.has_value();
}

LLVMFnArgInfo
LLVMFnInfo::sret() const
{
	return sret_arg.value();
}
