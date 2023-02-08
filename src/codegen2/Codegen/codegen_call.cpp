#include "codegen_call.h"

#include "../Codegen.h"
#include "lookup.h"

using namespace cg;

static llvm::Value*
promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	auto PointedToType = Val->getType()->getPointerElementType();
	return cg.Builder->CreateLoad(PointedToType, Val);
}

static llvm::Value*
__deprecate_promote_to_value(CG& cg, llvm::Value* Val)
{
	// TODO: Opaque pointers, don't rel
	if( Val->getType()->isPointerTy() )
		return promote_to_value(cg, Val);
	else
		return Val;
}

CGResult<CGExpr>
cg::codegen_call(CG& codegen, cg::LLVMFnSigInfo& fn, ir::IRCall* ir_call)
{
	return codegen_call(codegen, fn, ir_call, std::optional<LValue>());
}

CGResult<CGExpr>
cg::codegen_call(
	CG& codegen, cg::LLVMFnSigInfo& fn, ir::IRCall* ir_call, std::optional<LValue> lvalue)
{
	auto call_target_type = ir_call->call_target->type_instance;
	assert(call_target_type.type->is_function_type() && call_target_type.indirection_level <= 1);

	auto exprr = codegen.codegen_expr(fn, ir_call->call_target);
	if( !exprr.ok() )
		return exprr;

	auto expr = exprr.unwrap();
	if( expr.type != CGExprType::FunctionValue )
		return CGError("Call target is not a value??"); // Assert here?

	auto Function = expr.data.fn;
	auto iter_callee = codegen.Functions.find(Function->getName().str());
	assert(iter_callee != codegen.Functions.end());
	auto callee = iter_callee->second;

	auto return_type = call_target_type.type->get_return_type().value();

	// Assert value is correct?
	// Semantic analysis should've guaranteed this is a function.
	// if( !Value->getType()->isPointerTy() &&
	// 	!Value->getType()->getPointerElementType()->isFunctionTy() )
	// {
	// 	std::cout << "Expected function type" << std::endl;
	// 	return;
	// }

	std::vector<llvm::Value*> ArgsV;
	if( Function->getArg(0)->hasAttribute(llvm::Attribute::StructRet) )
	{
		// Call with sret
		auto rtr = get_type(codegen, return_type);
		if( !rtr.ok() )
			return rtr;
		auto SRetType = rtr.unwrap();

		if( lvalue.has_value() )
		{
			ArgsV.push_back(lvalue.value().value());
		}
		else
		{
			llvm::AllocaInst* SRetAlloca = codegen.Builder->CreateAlloca(SRetType, nullptr, "Wow");
			ArgsV.push_back(SRetAlloca);
		}

		return_type = sema::TypeInstance::OfType(codegen.sema.types.void_type());
	}

	int ir_arg_count = 0;
	for( auto arg_expr_node : *ir_call->args->args )
	{
		auto arg_exprr = codegen.codegen_expr(fn, arg_expr_node);
		if( !arg_exprr.ok() )
			return arg_exprr;

		auto arg_expr = arg_exprr.unwrap();
		if( arg_expr.type != CGExprType::Value )
			return CGError("Arg is not a value??"); // Assert here?

		auto ArgValue = arg_expr.data.value;

		auto arg_attr = ir_arg_count >= callee.ArgsTypes.size() && callee.is_var_arg
							? LLVMArgABIInfo::Default
							: callee.arg_type(ir_arg_count).attr;
		if( arg_attr == LLVMArgABIInfo::Value )
		{
			auto arg_type = callee.arg_type(ir_arg_count);
			llvm::AllocaInst* ArgAlloca = codegen.Builder->CreateAlloca(arg_type.type, nullptr);
			auto Size = codegen.Module->getDataLayout().getTypeAllocSize(
				arg_type.type->getPointerElementType());
			auto Align = codegen.Module->getDataLayout().getPrefTypeAlign(
				arg_type.type->getPointerElementType());

			codegen.Builder->CreateMemCpy(ArgAlloca, Align, ArgValue, Align, Size);
			ArgsV.push_back(ArgAlloca);
		}
		else
		{
			// TODO: Again, constants return as values, and allocas are pointers.
			// Need to consolidate this.
			auto ArgValuePromoted = ArgValue;
			// arg_expr.literal ? ArgValue : __deprecate_promote_to_value(codegen, ArgValue);

			ArgsV.push_back(ArgValuePromoted);
		}
		ir_arg_count += 1;
	}

	// https://github.com/ark-lang/ark/issues/362

	if( codegen.sema.types.equal_types(return_type, codegen.sema.types.VoidType()) )
	{
		auto CallValue = codegen.Builder->CreateCall(Function, ArgsV);
		return CGExpr();
	}
	else
	{
		auto CallValue = codegen.Builder->CreateCall(Function, ArgsV, "call");
		return CGExpr(CallValue);
	}
}