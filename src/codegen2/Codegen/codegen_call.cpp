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
cg::codegen_call(CG& codegen, cg::LLVMFnInfo& fn, ir::IRCall* ir_call)
{
	return codegen_call(codegen, fn, ir_call, std::optional<LValue>());
}

CGResult<CGExpr>
cg::codegen_call(CG& codegen, cg::LLVMFnInfo& fn, ir::IRCall* ir_call, std::optional<LValue> lvalue)
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
	auto callee_sig_info = iter_callee->second;

	std::vector<llvm::Value*> llvm_arg_values;
	int arg_ind = 0;
	if( callee_sig_info.has_sret_arg() )
	{
		// TODO: Assuming sret is first argument always!
		auto sret_arg_info = callee_sig_info.arg_type(callee_sig_info.sret_arg_index());
		if( lvalue.has_value() )
		{
			llvm_arg_values.push_back(lvalue.value().value());
		}
		else
		{
			// If no value was provided for the return value create a dummy alloca.
			llvm::AllocaInst* llvm_sret_alloca =
				codegen.Builder->CreateAlloca(sret_arg_info.llvm_type, nullptr, ".dummy");
			llvm_arg_values.push_back(llvm_sret_alloca);
		}
		arg_ind += 1;
	}

	for( auto arg_expr_node : *ir_call->args->args )
	{
		auto arg_exprr = codegen.codegen_expr(fn, arg_expr_node);
		if( !arg_exprr.ok() )
			return arg_exprr;

		auto arg_expr = arg_exprr.unwrap();
		if( arg_expr.type != CGExprType::Value )
			return CGError("Arg is not a value??"); // Assert here?

		auto llvm_arg_expr_value = arg_expr.data.value;

		auto abi_info = callee_sig_info.arg_type(arg_ind);

		switch( abi_info.attr )
		{
		case LLVMArgABIInfo::UncheckedVarArg:
		case LLVMArgABIInfo::Default:
		{
			// For unchecked args, the ABI doesn't have the arg type,
			// so we just use the type of the expression.
			// TODO: Need to return lvalues/rvalues that include the llvm type
			auto abi_llvm_type = abi_info.attr == LLVMArgABIInfo::UncheckedVarArg
									 ? !arg_expr.literal
										   ? llvm_arg_expr_value->getType()->getPointerElementType()
										   : llvm_arg_expr_value->getType()
									 : abi_info.llvm_type;
			// TODO: This is confusing.
			// When codegening literals, they return the direct value that we want.
			// When codegening non-literal, they return a pointer to the value we want.
			auto llvm_value = !arg_expr.literal
								  ? codegen.Builder->CreateLoad(abi_llvm_type, llvm_arg_expr_value)
								  : llvm_arg_expr_value;
			llvm_arg_values.push_back(llvm_value);
			break;
		}
		case LLVMArgABIInfo::Value:
		{
			// TODO: This is a bit confusing.
			// Passing struct by values actually passes a pointer.
			// We need to make a copy in a new alloca, and then pass
			// that alloca
			llvm::AllocaInst* llvm_cpy_alloca =
				codegen.Builder->CreateAlloca(abi_info.llvm_type->getPointerElementType(), nullptr);
			auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(
				abi_info.llvm_type->getPointerElementType());
			auto llvm_align = codegen.Module->getDataLayout().getPrefTypeAlign(
				abi_info.llvm_type->getPointerElementType());

			codegen.Builder->CreateMemCpy(
				llvm_cpy_alloca, llvm_align, llvm_arg_expr_value, llvm_align, llvm_size);

			llvm_arg_values.push_back(llvm_cpy_alloca);
			break;
		}
		case LLVMArgABIInfo::SRet:
		{
			assert(0 && "SRet ABI found in ir!");
			break;
		}
		}

		arg_ind += 1;
	}

	// https://github.com/ark-lang/ark/issues/362

	auto llvm_call = codegen.Builder->CreateCall(callee_sig_info.llvm_fn, llvm_arg_values);
	// If an sret arg was provided, then we have already turned the value.
	if( callee_sig_info.has_sret_arg() )
		return CGExpr();
	else
		return CGExpr(llvm_call);
}