#include "codegen_function.h"

#include "../Codegen.h"
#include "CGFunctionContext.h"
#include "lookup.h"

using namespace cg;

static cg::CGResult<cg::CGExpr>
codegen_function_entry_sret(CG& cg, cg::CGFunctionContext& ctx)
{
	assert(ctx.ret_type == CGFunctionContext::RetType::SRet);
	auto Function = ctx.Fn;
	auto fn_type = ctx.fn_type;

	auto args = Function->args();
	if( args.empty() )
		return CGExpr();

	// TODO: Need a better way to generate an SRet function
	int idx = 0;
	bool skipped = false;
	for( auto& Arg : args )
	{
		if( !skipped )
		{
			skipped = true;
			continue;
		}

		auto arg_info = fn_type->get_member(idx - 1);
		// Create an alloca for this variable.
		auto arg_name = arg_info.name;
		llvm::AllocaInst* Alloca = cg.Builder->CreateAlloca(Arg.getType(), nullptr, arg_name);

		// Store the initial value into the alloca.
		cg.Builder->CreateStore(&Arg, Alloca);

		cg.values.emplace(arg_name, Alloca);
		ctx.Args.push_back(Alloca);
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
	if( args.empty() )
		return CGExpr();

	auto idx = 0;

	for( auto& Arg : args )
	{
		auto arg_info = fn_type->get_member(idx);
		// Create an alloca for this variable.
		auto arg_name = arg_info.name;
		llvm::AllocaInst* Alloca = cg.Builder->CreateAlloca(Arg.getType(), nullptr, arg_name);

		// Store the initial value into the alloca.
		cg.Builder->CreateStore(&Arg, Alloca);

		cg.values.emplace(arg_name, Alloca);
		ctx.Args.push_back(Alloca);
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

CGResult<CGExpr>
cg::codegen_function(CG& cg, ir::IRFunction* fn)
{
	auto protor = codegen_function_proto(cg, fn->proto);
	if( !protor.ok() )
		return protor;

	cg.current_function = protor.unwrap();

	auto&& ctx = cg.current_function.value();
	auto entryr = codegen_function_entry(cg, ctx);
	if( !entryr.ok() )
		return entryr;

	auto bodyr = codegen_function_body(cg, fn->block, ctx);
	if( !bodyr.ok() )
		return bodyr;

	cg.current_function.reset();

	return protor;
}

static CGResult<CGFunctionContext>
codegen_function_proto_default(
	CG& codegen,
	ir::IRProto* proto,
	String* name,
	get_params_types_t& params_info,
	llvm::Type* ReturnTy)
{
	auto ParamsTys = params_info.types;
	auto is_var_arg = params_info.is_var_arg;
	llvm::FunctionType* FT = llvm::FunctionType::get(ReturnTy, ParamsTys, is_var_arg);

	llvm::Function* Function =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

	codegen.Functions.emplace(*name, Function);
	auto fn_type = proto->fn_type;
	codegen.types.emplace(fn_type, FT);
	codegen.values.emplace(*name, Function);

	return CGFunctionContext(Function, FT, fn_type, CGFunctionContext::RetType::Default);
}

static CGResult<CGFunctionContext>
codegen_function_proto_sret(
	CG& codegen,
	ir::IRProto* proto,
	String* name,
	get_params_types_t& params_info,
	llvm::Type* ReturnTy)
{
	auto ParamsTys = params_info.types;
	auto is_var_arg = params_info.is_var_arg;

	ParamsTys.insert(ParamsTys.begin(), ReturnTy->getPointerTo());
	auto VoidReturnTy = llvm::Type::getVoidTy(*codegen.Context);

	llvm::FunctionType* FT = llvm::FunctionType::get(VoidReturnTy, ParamsTys, is_var_arg);

	llvm::Function* Function =
		llvm::Function::Create(FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

	// Function->getArg(0)->addAttr(
	// 	llvm::Attribute::get(*Context, llvm::Attribute::AttrKind::StructRet));
	auto& Builder = llvm::AttrBuilder().addStructRetAttr(ReturnTy);

	Function->getArg(0)->addAttrs(Builder);

	codegen.Functions.emplace(*name, Function);
	auto fn_type = proto->fn_type;
	codegen.types.emplace(fn_type, FT);
	codegen.values.emplace(*name, Function);

	return CGFunctionContext(Function, FT, fn_type, CGFunctionContext::RetType::SRet);
}

CGResult<CGFunctionContext>
cg::codegen_function_proto(CG& codegen, ir::IRProto* proto)
{
	auto name = proto->name;

	auto paramsr = get_params_types(codegen, proto);
	if( !paramsr.ok() )
		return paramsr;
	auto params_info = paramsr.unwrap();
	auto ParamsTys = params_info.types;
	auto is_var_arg = params_info.is_var_arg;

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
cg::codegen_function_body(CG& cg, ir::IRBlock* block, cg::CGFunctionContext& ctx)
{
	for( auto stmt : *block->stmts )
	{
		//
		auto stmtr = cg.codegen_stmt(stmt);
		if( !stmtr.ok() )
			return stmtr;
	}

	return CGExpr();
}