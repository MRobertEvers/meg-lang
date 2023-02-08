#pragma once

#include "LLVMFnInfo.h"
#include "LLVMFnSigInfo.h"
#include "common/String.h"

#include <map>
#include <optional>

namespace cg
{

struct LLVMFnInfoBuilder
{
	LLVMFnSigInfo sig_info;

	std::map<String, LLVMFnArgInfo> args;

	std::optional<LLVMFnArgInfo> sret_arg;

	LLVMFnInfoBuilder(LLVMFnSigInfo sig_info);

	void add_arg(LLVMFnArgInfo arg_info);

	LLVMFnInfo to_fn_info() const;
};
} // namespace cg