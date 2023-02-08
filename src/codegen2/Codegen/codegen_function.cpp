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
	Vec<std::pair<String, LLVMArgABIInfo>> args;
	bool is_var_arg;
};

static CGResult<get_params_types_t>
get_named_params(CG& cg, ir::IRProto* proto)
{
	get_params_types_t args;
	args.is_var_arg = false;

	for( auto& arg : *proto->args )
	{
		if( arg->type == ir::IRParamType::ValueDecl )
		{
			auto ir_value_decl = arg->data.value_decl;
			auto argsr = get_type(cg, ir_value_decl);
			if( !argsr.ok() )
				return argsr;
			auto llvm_arg_ty = argsr.unwrap();
			auto name = *ir_value_decl->name;
			auto sema_ty = ir_value_decl->type_decl->type_instance;
			if( sema_ty.is_struct_type() )
			{
				args.args.emplace_back(
					String(name),
					LLVMArgABIInfo(LLVMArgABIInfo::Value, llvm_arg_ty->getPointerTo()));
			}
			else
			{
				args.args.emplace_back(
					String(name), LLVMArgABIInfo(LLVMArgABIInfo::Default, llvm_arg_ty));
			}
		}
		else
		{
			args.is_var_arg = true;
		}
	}

	return args;
}

// static cg::CGResult<cg::CGExpr>
// codegen_function_entry_param(
// 	CG& cg, cg::LLVMFnInfo& ctx, String const& name, llvm::Argument* Arg, LLVMArgABIInfo ArgType)
// {
// 	if( ArgType.attr == LLVMArgABIInfo::Value )
// 	{
// 		ctx.add_lvalue(name, LValue(Arg, ArgType.type));

// 		cg.values.emplace(name, Arg);
// 	}
// 	else
// 	{
// 		llvm::AllocaInst* Alloca = cg.Builder->CreateAlloca(ArgType.type, nullptr, name);
// 		cg.Builder->CreateStore(Arg, Alloca);

// 		ctx.add_lvalue(name, LValue(Alloca, ArgType.type));

// 		// TODO: Remove this once exprs get ctx arg
// 		cg.values.emplace(name, Alloca);
// 	}

// 	return CGExpr();
// }

// static cg::CGResult<cg::CGExpr>
// codegen_function_entry_sret(CG& cg, cg::LLVMFnInfo& ctx)
// {
// 	assert(ctx.ret_type == LLVMFnSigInfo::RetType::SRet);
// 	auto Function = ctx.Fn;
// 	auto fn_type = ctx.fn_type;

// 	// SRet should always have at least one arg.
// 	auto args = Function->args();
// 	assert(!args.empty());

// 	int idx = 0;
// 	bool skipped = false;
// 	for( auto& Arg : args )
// 	{
// 		if( !skipped )
// 		{
// 			skipped = true;
// 			continue;
// 		}

// 		auto arg_info = fn_type->get_member(idx);
// 		auto ArgType = ctx.arg_type(idx + 1);
// 		auto arg_name = arg_info.name;

// 		auto genr = codegen_function_entry_param(cg, ctx, arg_name, &Arg, ArgType);
// 		if( !genr.ok() )
// 			return genr;

// 		idx++;
// 	}

// 	return CGExpr();
// }

// static cg::CGResult<cg::CGExpr>
// codegen_function_entry_default(CG& cg, cg::LLVMFnInfo& ctx)
// {
// 	auto Function = ctx.Fn;
// 	auto fn_type = ctx.fn_type;

// 	auto args = Function->args();

// 	auto idx = 0;
// 	for( auto& Arg : args )
// 	{
// 		auto arg_info = fn_type->get_member(idx);
// 		auto ArgType = ctx.arg_type(idx);
// 		auto arg_name = arg_info.name;

// 		auto genr = codegen_function_entry_param(cg, ctx, arg_name, &Arg, ArgType);
// 		if( !genr.ok() )
// 			return genr;

// 		idx++;
// 	}

// 	return CGExpr();
// }

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

			llvm::AllocaInst* llvm_alloca =
				codegen.Builder->CreateAlloca(arg_abi.llvm_type, nullptr, fn_info.name);
			codegen.Builder->CreateStore(llvm_arg, llvm_alloca);

			builder.add_arg(
				LLVMFnArgInfo::Named(name, arg_abi, LValue(llvm_alloca, arg_abi.llvm_type)));

			// TODO: Remove this once exprs get ctx arg
			codegen.values.emplace(name, llvm_alloca);
			break;
		}
		case LLVMArgABIInfo::SRet:
		{
			// TODO: Opaque pointer.
			builder.add_arg(LLVMFnArgInfo::SRet(arg_abi, LValue(llvm_arg, arg_abi.llvm_type)));
			break;
		}
		case LLVMArgABIInfo::Value:
		{
			auto maybe_name = fn_info.get_arg_name(arg_ind);
			assert(maybe_name.has_value());
			auto name = maybe_name.value();

			// TODO: Make sure call actually provides a memcpy
			codegen.values.emplace(name, llvm_arg);
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

	cg.current_function = fn_sig_info;
	auto entryr = codegen_function_entry(cg, fn_sig_info);
	if( !entryr.ok() )
		return entryr;
	auto fn_info = entryr.unwrap();

	auto bodyr = codegen_function_body(cg, fn_info, ir_fn->block);
	if( !bodyr.ok() )
		return bodyr;

	cg.current_function.reset();

	return protor;
}

static Vec<llvm::Type*>
from_argtypes(Vec<LLVMArgABIInfo>& args)
{
	Vec<llvm::Type*> vec;
	for( auto arg : args )
	{
		vec.push_back(arg.llvm_type);
	}
	return vec;
}

// static CGResult<LLVMFnSigInfo>
// codegen_function_proto_default(
// 	CG& codegen,
// 	ir::IRProto* proto,
// 	String* name,
// 	get_params_types_t& params_info,
// 	llvm::Type* ReturnTy)
// {
// 	auto args = params_info.args;
// 	auto ParamsTys = from_argtypes(args);
// 	auto is_var_arg = params_info.is_var_arg;
// 	llvm::FunctionType* FT = llvm::FunctionType::get(ReturnTy, ParamsTys, is_var_arg);

// 	llvm::Function* Function =
// 		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

// 	for( int i = 0; i < ParamsTys.size(); i++ )
// 	{
// 		auto Arg = Function->getArg(i);
// 		auto arg_type = args.at(i);
// 		if( arg_type.attr == LLVMArgABIInfo::Value )
// 		{
// 			auto& Builder = llvm::AttrBuilder().addStructRetAttr(arg_type.type);
// 			Arg->addAttrs(Builder);
// 		}
// 	}

// 	auto fn_type = proto->fn_type;

// 	auto context =
// 		LLVMFnSigInfo(Function, FT, args, is_var_arg, fn_type, LLVMFnSigInfo::RetType::Default);
// 	codegen.add_function(*name, context);

// 	return context;
// }

// static CGResult<LLVMFnSigInfo>
// codegen_function_proto_sret(
// 	CG& codegen,
// 	ir::IRProto* proto,
// 	String* name,
// 	get_params_types_t& params_info,
// 	llvm::Type* llvm_return_ty)
// {
// 	auto args = params_info.args;
// 	auto llvm_param_tys = from_argtypes(args);
// 	auto is_var_arg = params_info.is_var_arg;

// 	// TODO: Opaque pointer.
// 	args.insert(
// 		args.begin(), LLVMArgABIInfo(LLVMArgABIInfo::Default, llvm_return_ty->getPointerTo()));
// 	llvm_param_tys.insert(llvm_param_tys.begin(), llvm_return_ty->getPointerTo());
// 	auto llvm_void_ty = llvm::Type::getVoidTy(*codegen.Context);

// 	llvm::FunctionType* llvm_fn_ty =
// 		llvm::FunctionType::get(llvm_void_ty, llvm_param_tys, is_var_arg);

// 	llvm::Function* llvm_fn = llvm::Function::Create(
// 		llvm_fn_ty, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

// 	auto& llvm_attr_builder = llvm::AttrBuilder().addStructRetAttr(llvm_return_ty);
// 	llvm_fn->getArg(0)->addAttrs(llvm_attr_builder);

// 	for( int i = 0; i < llvm_param_tys.size(); i++ )
// 	{
// 		auto llvm_arg = llvm_fn->getArg(i);
// 		auto arg_type = args.at(i);
// 		if( arg_type.attr == LLVMArgABIInfo::Value )
// 		{
// 			llvm_arg->addAttrs(llvm::AttrBuilder().addByValAttr(arg_type.type));
// 		}
// 	}

// 	auto fn_type = proto->fn_type;
// 	auto context =
// 		LLVMFnSigInfo(llvm_fn, llvm_fn_ty, args, is_var_arg, fn_type, LLVMFnSigInfo::RetType::SRet);
// 	codegen.add_function(*name, context);

// 	return context;
// }

CGResult<LLVMFnSigInfo>
cg::codegen_function_proto(CG& codegen, ir::IRProto* ir_proto)
{
	auto name = ir_proto->name;

	auto paramsr = get_named_params(codegen, ir_proto);
	if( !paramsr.ok() )
		return paramsr;
	auto params_info = paramsr.unwrap();

	auto ir_rt_decl = ir_proto->rt;
	auto retr = get_type(codegen, ir_rt_decl);
	if( !retr.ok() )
		return retr;

	auto sema_rt_ty = ir_rt_decl->type_instance;
	auto llvm_rt_ty = retr.unwrap();

	LLVMFnSigInfoBuilder builder(*name, ir_proto->fn_type);

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
	{
		builder.add_arg_type(arg_name, arg_abi);
	}

	builder.set_is_var_arg(params_info.is_var_arg);

	auto sig_info = codegen_fn_sig_info(codegen, builder);

	codegen.add_function(*name, sig_info);

	return sig_info;
}

CGResult<CGExpr>
cg::codegen_function_body(CG& cg, cg::LLVMFnInfo& ctx, ir::IRBlock* block)
{
	for( auto stmt : *block->stmts )
	{
		//
		auto stmtr = cg.codegen_stmt(ctx, stmt);
		if( !stmtr.ok() )
			return stmtr;
	}

	return CGExpr();
}