#pragma once

#include "../Codegen.h"
#include "LLVMFnSigInfo.h"
#include "LLVMFnSigInfoBuilder.h"

namespace cg
{
LLVMFnSigInfo codegen_fn_sig_info(CG&, LLVMFnSigInfoBuilder const& builder);
}