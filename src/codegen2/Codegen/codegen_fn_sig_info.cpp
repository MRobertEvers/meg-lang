#include "codegen_fn_sig_info.h"

#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

cg::LLVMFnSigInfo
cg::codegen_fn_sig_info(CG& codegen, LLVMFnSigInfoBuilder const& builder)
{
	llvm::FunctionType* llvm_fn_ty = llvm::FunctionType::get(
		builder.llvm_ret_ty, get_llvm_arg_types(builder.abi_arg_infos), builder.is_var_arg());

	llvm::Function* llvm_fn = llvm::Function::Create(
		llvm_fn_ty, llvm::Function::ExternalLinkage, builder.name, codegen.Module.get());

	int arg_ind = 0;
	for( auto& abi_arg : builder.abi_arg_infos )
	{
		auto llvm_arg = llvm_fn->getArg(arg_ind);

		switch( abi_arg.attr )
		{
		case LLVMArgABIInfo::UncheckedVarArg:
		case LLVMArgABIInfo::Default:
			break;
		case LLVMArgABIInfo::SRet:
		{
			llvm_arg->addAttrs(llvm::AttrBuilder().addStructRetAttr(abi_arg.llvm_type));
			break;
		}
		case LLVMArgABIInfo::Value:
		{
			llvm_arg->addAttrs(llvm::AttrBuilder().addByValAttr(abi_arg.llvm_type));
			break;
		}
		}

		arg_ind++;
	}

	return LLVMFnSigInfo(
		builder.name,
		llvm_fn,
		llvm_fn_ty,
		builder.abi_arg_infos,
		builder.named_args_info_inds,
		builder.sema_fn_ty,
		builder.ret_type,
		builder.has_sret_arg() ? builder.sret_arg_index() : 0,
		builder.is_var_arg());
}