#include "codegen_function.h"

#include "../Codegen.h"
#include "LLVMFnInfo.h"
#include "LLVMFnInfoBuilder.h"
#include "LLVMFnSigInfo.h"
#include "LLVMFnSigInfoBuilder.h"
#include "codegen_fn_sig_info.h"
#include "lookup.h"

#include <utility>
using namespace cg;

struct get_params_types_t
{
	Vec<std::pair<sema::NameRef, LLVMArgABIInfo>> args;
	bool is_var_arg;
};

static CGResult<get_params_types_t>
get_named_params(CG& cg, ir::IRProto* proto)
{
	get_params_types_t args;
	args.is_var_arg = false;

	for( auto& arg : proto->args )
	{
		if( arg.kind == ir::ProtoArg::Kind::Named )
		{
			sema::NameRef name_ref = arg.name;
			auto argsr = get_type(cg, name_ref.type());
			if( !argsr.ok() )
				return argsr;
			auto llvm_arg_ty = argsr.unwrap();
			auto name = name_ref.name().name_str();
			auto sema_ty = name_ref.type();
			if( sema_ty.is_struct_type() || sema_ty.is_enum_type() || sema_ty.is_union_type() )
			{
				args.args.emplace_back(
					name_ref, LLVMArgABIInfo(LLVMArgABIInfo::Value, llvm_arg_ty->getPointerTo()));
			}
			else
			{
				args.args.emplace_back(
					name_ref, LLVMArgABIInfo(LLVMArgABIInfo::Default, llvm_arg_ty));
			}
		}
		else
		{
			args.is_var_arg = true;
		}
	}

	return args;
}

static cg::CGResult<LLVMFnInfo>
codegen_function_entry(CG& codegen, cg::LLVMFnSigInfo& fn_info)
{
	LLVMFnInfoBuilder builder(fn_info);

	llvm::BasicBlock* llvm_entry_bb =
		llvm::BasicBlock::Create(*codegen.Context, "entry", fn_info.llvm_fn);
	codegen.Builder->SetInsertPoint(llvm_entry_bb);

	int arg_ind = 0;
	for( auto& arg_abi : fn_info.abi_arg_infos )
	{
		auto llvm_arg = fn_info.llvm_fn->getArg(arg_ind);

		switch( arg_abi.attr )
		{
		case LLVMArgABIInfo::UncheckedVarArg:
		case LLVMArgABIInfo::Default:
		{
			auto maybe_name = fn_info.get_arg_name(arg_ind);
			assert(maybe_name.has_value());
			auto name = maybe_name.value();

			llvm::AllocaInst* llvm_alloca = codegen.builder_alloca(arg_abi.llvm_type);
			codegen.Builder->CreateAlloca(arg_abi.llvm_type, nullptr, name.name().name_str());
			codegen.Builder->CreateStore(llvm_arg, llvm_alloca);

			auto lvalue = LValue(llvm_alloca, arg_abi.llvm_type);
			builder.add_arg(LLVMFnArgInfo::Named(name, arg_abi, lvalue));

			break;
		}
		case LLVMArgABIInfo::SRet:
		{
			// TODO: Opaque pointer.
			builder.add_arg(LLVMFnArgInfo::SRet(
				arg_abi, LValue(llvm_arg, arg_abi.llvm_type->getPointerElementType())));
			break;
		}
		case LLVMArgABIInfo::Value:
		{
			auto maybe_name = fn_info.get_arg_name(arg_ind);
			assert(maybe_name.has_value());
			auto name = maybe_name.value();

			auto lvalue = LValue(llvm_arg, llvm_arg->getType()->getPointerElementType());

			builder.add_arg(LLVMFnArgInfo::Named(name, arg_abi, lvalue));
			break;
		}
		}

		arg_ind++;
	}

	return builder.to_fn_info();
}

/**
 * @brief
 *
 *
 * @param cg
 * @param ir_fn
 * @return CGResult<CGExpr>
 */
CGResult<CGExpr>
cg::codegen_function(CG& cg, ir::IRFunction* ir_fn)
{
	auto protor = codegen_function_proto(cg, ir_fn->proto);
	if( !protor.ok() )
		return protor;

	auto fn_sig_info = protor.unwrap();

	auto entryr = codegen_function_entry(cg, fn_sig_info);
	if( !entryr.ok() )
		return entryr;
	auto fn_info = entryr.unwrap();

	auto bodyr = codegen_function_body(cg, fn_info, ir_fn->block);
	if( !bodyr.ok() )
		return bodyr;

	for( llvm::BasicBlock* block : fn_info.blocks_ )
		fn_sig_info.llvm_fn->getBasicBlockList().push_back(block);

	return protor;
}

static Vec<llvm::Type*>
from_argtypes(Vec<LLVMArgABIInfo>& args)
{
	Vec<llvm::Type*> vec;
	for( auto arg : args )
		vec.push_back(arg.llvm_type);
	return vec;
}

CGResult<LLVMFnSigInfo>
cg::codegen_function_proto(CG& codegen, ir::IRProto* ir_proto)
{
	auto name = ir_proto->name;

	auto params_result = get_named_params(codegen, ir_proto);
	if( !params_result.ok() )
		return params_result;
	auto params_info = params_result.unwrap();

	auto ir_rt_decl = ir_proto->rt;
	auto retr = get_type(codegen, ir_rt_decl);
	if( !retr.ok() )
		return retr;

	auto sema_rt_ty = ir_rt_decl->type_instance;
	auto llvm_rt_ty = retr.unwrap();

	LLVMFnSigInfoBuilder builder(name, ir_proto->fn_type);

	if( sema_rt_ty.is_struct_type() )
	{
		// Promote the return type to an SRet param and change the return type to void.
		// TODO: Opaque pointer
		builder.add_arg_type(LLVMArgABIInfo(LLVMArgABIInfo::SRet, llvm_rt_ty->getPointerTo()));
		builder.set_llvm_ret_ty(llvm::Type::getVoidTy(*codegen.Context));
	}
	else
	{
		builder.set_llvm_ret_ty(llvm_rt_ty);
	}

	for( auto& [arg_name, arg_abi] : params_info.args )
		builder.add_arg_type(arg_name, arg_abi);

	builder.set_is_var_arg(params_info.is_var_arg);

	auto sig_info = codegen_fn_sig_info(codegen, builder);

	codegen.add_function(name.type().type, sig_info);

	return sig_info;
}

CGResult<CGExpr>
cg::codegen_function_body(CG& cg, cg::LLVMFnInfo& ctx, ir::IRBlock* block)
{
	auto previous_scope = cg.values;

	for( auto [name_id, arg] : ctx.named_args )
		cg.values.insert_or_assign(name_id, arg.lvalue);

	auto block_return = cg.codegen_block(ctx, block);

	cg.values = previous_scope;

	return block_return;
}