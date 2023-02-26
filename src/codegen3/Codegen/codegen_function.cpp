#include "codegen_function.h"

#include "../Codegen.h"
#include "LLVMFnInfo.h"
#include "LLVMFnInfoBuilder.h"
#include "LLVMFnSigInfo.h"
#include "LLVMFnSigInfoBuilder.h"
#include "codegen_fn_sig_info.h"
#include "codegen_inst.h"
#include "ir/Type.h"
#include "lookup.h"

#include <string>
#include <utility>
#include <vector>

using namespace cg;

struct get_params_types_t
{
	std::vector<std::pair<std::string, LLVMArgABIInfo>> args;
	bool is_var_arg;
};

static get_params_types_t
get_named_params(CG& cg, ir::TypeInstance fn_type)
{
	get_params_types_t args;
	args.is_var_arg = fn_type.type->is_var_arg();
	auto type = fn_type.type;

	for( int i = 0; i < type->get_member_count(); i++ )
	{
		auto arg = type->get_member(i);

		auto llvm_arg_ty = get_type(cg, arg.type).value();
		auto name = arg.name;

		auto sema_ty = arg.type;
		if( sema_ty.is_struct_type() || sema_ty.is_enum_type() || sema_ty.is_union_type() )
		{
			args.args.emplace_back(
				std::string(name),
				LLVMArgABIInfo(LLVMArgABIInfo::Value, llvm_arg_ty->getPointerTo()));
		}
		else
		{
			args.args.emplace_back(
				std::string(name), LLVMArgABIInfo(LLVMArgABIInfo::Default, llvm_arg_ty));
		}
	}

	return args;
}

static LLVMFnInfo
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

			llvm::AllocaInst* llvm_alloca =
				codegen.Builder->CreateAlloca(arg_abi.llvm_type, nullptr, name);
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

static std::vector<llvm::Type*>
from_argtypes(std::vector<LLVMArgABIInfo>& args)
{
	std::vector<llvm::Type*> vec;
	for( auto arg : args )
	{
		vec.push_back(arg.llvm_type);
	}
	return vec;
}

static LLVMFnSigInfo
codegen_function_type(CG& codegen, ir::NameId id, ir::TypeInstance type)
{
	auto name = ir::NameRef(&codegen.names, id).to_fqn_string();

	auto params_info = get_named_params(codegen, type);

	auto sema_rt_ty = type.type->get_return_type().value();
	auto llvm_rt_ty = get_type(codegen, sema_rt_ty).value();

	LLVMFnSigInfoBuilder builder(name, type.type);

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

	codegen.add_function(id.index(), sig_info);

	return sig_info;
}

CGExpr
cg::codegen_function_proto(CG& codegen, ir::FnDecl* ir_proto)
{
	codegen_function_type(codegen, ir_proto->name_id, ir_proto->type);
	return CGExpr();
}

/**
 * @brief
 *
 *
 * @param cg
 * @param ir_fn
 * @return CGResult<CGExpr>
 */
CGExpr
cg::codegen_function(CG& codegen, ir::Function* ir_fn)
{
	auto fn_sig_info = codegen.get_function(ir_fn->type.type);

	auto fn_info = codegen_function_entry(codegen, fn_sig_info);

	for( auto bb : ir_fn->blocks )
	{
		for( auto inst : bb->instructions )
			codegen_inst(codegen, inst);
	}
	// auto fn_sig_info = codegen_function_proto(cg, ir_fn->proto);
	// if( !protor.ok() )
	// 	return protor;

	// auto fn_sig_info = protor.unwrap();

	// auto bodyr = codegen_function_body(cg, fn_info, ir_fn->block);
	// if( !bodyr.ok() )
	// 	return bodyr;

	return CGExpr();
}

// static CGExpr
// codegen_function_body(CG& cg, cg::LLVMFnInfo& ctx, ir::* block)
// {
// 	// auto previous_scope = cg.values;

// 	// for( auto [name, arg] : ctx.named_args )
// 	// {
// 	// 	cg.values.insert_or_assign(name, arg.lvalue);
// 	// }

// 	// auto block_return = cg.codegen_block(ctx, block);

// 	// cg.values = previous_scope;

// 	// return block_return;
// 	return CGExpr();
// }