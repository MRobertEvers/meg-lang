#include "codegen_function.h"

#include "../Codegen.h"
#include "CGFunctionContext.h"
#include "lookup.h"

using namespace cg;

static cg::CGResult<cg::CGExpr>
codegen_function_entry_param(
	CG& cg,
	cg::CGFunctionContext& ctx,
	String const& name,
	llvm::Argument* Arg,
	llvm::Type* ArgType)
{
	// if( ArgType->isStructTy() )
	// {
	// 	ctx.add_lvalue(name, LValue(Arg, ArgType));
	// 	cg.values.emplace(name, Arg);
	// }
	// else
	// {
	llvm::AllocaInst* Alloca = cg.Builder->CreateAlloca(ArgType, nullptr, name);
	cg.Builder->CreateStore(Arg, Alloca);

	ctx.add_lvalue(name, LValue(Alloca, ArgType));

	// TODO: Remove this once exprs get ctx arg
	cg.values.emplace(name, Alloca);
	// }

	return CGExpr();
}

static cg::CGResult<cg::CGExpr>
codegen_function_entry_sret(CG& cg, cg::CGFunctionContext& ctx)
{
	assert(ctx.ret_type == CGFunctionContext::RetType::SRet);
	auto Function = ctx.Fn;
	auto fn_type = ctx.fn_type;

	// SRet should always have at least one arg.
	auto args = Function->args();
	assert(!args.empty());

	int idx = 0;
	bool skipped = false;
	for( auto& Arg : args )
	{
		if( !skipped )
		{
			skipped = true;
			continue;
		}

		auto arg_info = fn_type->get_member(idx);
		auto ArgType = ctx.arg_type(idx + 1);
		auto arg_name = arg_info.name;

		auto genr = codegen_function_entry_param(cg, ctx, arg_name, &Arg, ArgType.type);
		if( !genr.ok() )
			return genr;

		idx++;
	}

	return CGExpr();
}

static cg::CGResult<cg::CGExpr>
codegen_function_entry_default(CG& cg, cg::CGFunctionContext& ctx)
{
	assert(ctx.ret_type == CGFunctionContext::RetType::Default);
	auto Function = ctx.Fn;
	auto fn_type = ctx.fn_type;

	auto args = Function->args();

	auto idx = 0;
	for( auto& Arg : args )
	{
		auto arg_info = fn_type->get_member(idx);
		auto ArgType = ctx.arg_type(idx);
		auto arg_name = arg_info.name;

		auto genr = codegen_function_entry_param(cg, ctx, arg_name, &Arg, ArgType.type);
		if( !genr.ok() )
			return genr;

		idx++;
	}

	return CGExpr();
}

static cg::CGResult<cg::CGExpr>
codegen_function_entry(CG& cg, cg::CGFunctionContext& ctx)
{
	auto Function = ctx.Fn;

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(*cg.Context, "entry", Function);
	cg.Builder->SetInsertPoint(BB);

	if( ctx.ret_type == CGFunctionContext::RetType::Default )
	{
		return codegen_function_entry_default(cg, ctx);
	}
	else
	{
		return codegen_function_entry_sret(cg, ctx);
	}
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

	auto fn = protor.unwrap();

	cg.current_function = fn;
	auto entryr = codegen_function_entry(cg, fn);
	if( !entryr.ok() )
		return entryr;

	auto bodyr = codegen_function_body(cg, fn, ir_fn->block);
	if( !bodyr.ok() )
		return bodyr;

	cg.current_function.reset();

	return protor;
}

static Vec<llvm::Type*>
from_argtypes(Vec<ArgumentType>& args)
{
	Vec<llvm::Type*> vec;
	for( auto arg : args )
	{
		vec.push_back(arg.type);
	}
	return vec;
}

static CGResult<CGFunctionContext>
codegen_function_proto_default(
	CG& codegen,
	ir::IRProto* proto,
	String* name,
	get_params_types_t& params_info,
	llvm::Type* ReturnTy)
{
	auto args = params_info.args;
	auto ParamsTys = from_argtypes(args);
	auto is_var_arg = params_info.is_var_arg;
	llvm::FunctionType* FT = llvm::FunctionType::get(ReturnTy, ParamsTys, is_var_arg);

	llvm::Function* Function =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

	for( int i = 0; i < ParamsTys.size(); i++ )
	{
		auto Arg = Function->getArg(i);
		auto arg_type = args.at(i);
		if( arg_type.attr == ArgumentAttr::Value )
		{
			auto& Builder = llvm::AttrBuilder().addStructRetAttr(arg_type.type);
			Arg->addAttrs(Builder);
		}
	}

	auto fn_type = proto->fn_type;

	auto context =
		CGFunctionContext(Function, FT, args, fn_type, CGFunctionContext::RetType::Default);
	codegen.add_function(*name, context);

	return context;
}

/**
 * @brief
 *
 * 1.
 *
 * @param codegen
 * @param proto
 * @param name
 * @param params_info
 * @param ReturnTy
 * @return CGResult<CGFunctionContext>
 */

static CGResult<CGFunctionContext>
codegen_function_proto_sret(
	CG& codegen,
	ir::IRProto* proto,
	String* name,
	get_params_types_t& params_info,
	llvm::Type* ReturnTy)
{
	auto args = params_info.args;
	auto ParamsTys = from_argtypes(args);
	auto is_var_arg = params_info.is_var_arg;

	// TODO: Opaque pointer.
	args.insert(args.begin(), ArgumentType(ArgumentAttr::Default, ReturnTy->getPointerTo()));
	ParamsTys.insert(ParamsTys.begin(), ReturnTy->getPointerTo());
	auto VoidReturnTy = llvm::Type::getVoidTy(*codegen.Context);

	llvm::FunctionType* FT = llvm::FunctionType::get(VoidReturnTy, ParamsTys, is_var_arg);

	llvm::Function* Function =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

	auto& Builder = llvm::AttrBuilder().addStructRetAttr(ReturnTy);
	Function->getArg(0)->addAttrs(Builder);

	for( int i = 0; i < ParamsTys.size(); i++ )
	{
		auto Arg = Function->getArg(i);
		auto arg_type = args.at(i);
		// if( arg_type.attr == ArgumentAttr::Value )
		// {
		// 	Arg->addAttrs(llvm::AttrBuilder().addByValAttr(arg_type.type));
		// }
	}

	auto fn_type = proto->fn_type;
	auto context = CGFunctionContext(Function, FT, args, fn_type, CGFunctionContext::RetType::SRet);
	codegen.add_function(*name, context);

	return context;
}

CGResult<CGFunctionContext>
cg::codegen_function_proto(CG& codegen, ir::IRProto* proto)
{
	auto name = proto->name;

	auto paramsr = get_params_types(codegen, proto);
	if( !paramsr.ok() )
		return paramsr;
	auto params_info = paramsr.unwrap();

	auto retr = get_type(codegen, proto->rt);
	if( !retr.ok() )
		return retr;
	auto ReturnTy = retr.unwrap();

	auto rt_ty = proto->rt->type_instance;
	if( rt_ty.type->is_struct_type() && rt_ty.indirection_level == 0 )
	{
		return codegen_function_proto_sret(codegen, proto, name, params_info, ReturnTy);
	}
	else
	{
		return codegen_function_proto_default(codegen, proto, name, params_info, ReturnTy);
	}
}

CGResult<CGExpr>
cg::codegen_function_body(CG& cg, cg::CGFunctionContext& ctx, ir::IRBlock* block)
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