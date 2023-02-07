#include "codegen_function.h"

#include "../Codegen.h"
#include "CGFunctionContext.h"
#include "lookup.h"

using namespace cg;

static cg::CGResult<cg::CGExpr>
codegen_function_entry(CG& cg, cg::CGFunctionContext& ctx)
{
	auto Function = ctx.Fn;
	auto fn_type = ctx.fn_type;

	llvm::BasicBlock* BB = llvm::BasicBlock::Create(*cg.Context, "entry", Function);
	cg.Builder->SetInsertPoint(BB);

	auto args = Function->args();
	if( args.empty() )
		return CGExpr();

	// TODO: Need a better way to generate an SRet function
	auto idx = 0;
	auto skip_first = Function->getArg(0)->hasAttribute(llvm::Attribute::StructRet) ? 1 : 0;
	idx += skip_first;
	bool skipped = false;

	for( auto& Arg : args )
	{
		if( skip_first == 1 && !skipped )
		{
			skipped = true;
			continue;
		}

		auto arg_info = fn_type->get_member(idx - skip_first);
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

CGResult<CGExpr>
cg::codegen_function(CG& cg, ir::IRFunction* fn)
{
	auto protor = codegen_function_proto(cg, fn->proto);
	if( !protor.ok() )
		return protor;

	cg.current_function = protor.unwrap();

	auto&& val = cg.current_function.value();
	auto entryr = codegen_function_entry(cg, val);
	if( !entryr.ok() )
		return entryr;

	auto bodyr = codegen_function_body(cg, fn->block);
	if( !bodyr.ok() )
		return bodyr;

	cg.current_function.reset();

	return protor;
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
		ParamsTys.insert(ParamsTys.begin(), ReturnTy->getPointerTo());
		auto VoidReturnTy = llvm::Type::getVoidTy(*codegen.Context);

		llvm::FunctionType* FT = llvm::FunctionType::get(VoidReturnTy, ParamsTys, is_var_arg);

		llvm::Function* Function = llvm::Function::Create(
			FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

		// Function->getArg(0)->addAttr(
		// 	llvm::Attribute::get(*Context, llvm::Attribute::AttrKind::StructRet));
		auto& Builder = llvm::AttrBuilder().addStructRetAttr(ReturnTy);

		Function->getArg(0)->addAttrs(Builder);

		codegen.Functions.emplace(*name, Function);
		auto fn_type = proto->fn_type;
		codegen.types.emplace(fn_type, FT);
		codegen.values.emplace(*name, Function);

		return CGFunctionContext(Function, fn_type);
	}
	else
	{
		llvm::FunctionType* FT = llvm::FunctionType::get(ReturnTy, ParamsTys, is_var_arg);

		llvm::Function* Function = llvm::Function::Create(
			FT, llvm::Function::ExternalLinkage, *name, codegen.Module.get());

		codegen.Functions.emplace(*name, Function);
		auto fn_type = proto->fn_type;
		codegen.types.emplace(fn_type, FT);
		codegen.values.emplace(*name, Function);

		return CGFunctionContext(Function, fn_type);
	}
}

CGResult<CGExpr>
cg::codegen_function_body(CG& cg, ir::IRBlock* block)
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