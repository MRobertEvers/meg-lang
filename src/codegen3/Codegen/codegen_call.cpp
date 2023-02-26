#include "codegen_call.h"

#include "../Codegen.h"
#include "LLVMAddress.h"
#include "codegen_inst.h"
#include "lookup.h"
#include "operand.h"
#include <llvm/IR/IRBuilder.h>

using namespace cg;

CGExpr
cg::codegen_call(CG& codegen, ir::FnCall* ir_call)
{
	return codegen_call(codegen, ir_call, std::optional<LValue>());
}

CGExpr
cg::codegen_call(CG& codegen, ir::FnCall* ir_call, std::optional<LValue> lvalue)
{
	auto call_target_type = ir_call->type;
	assert(call_target_type.type->is_function_type() && call_target_type.indirection_level <= 1);

	// TODO: EmitCallee like clang.
	auto call_target = codegen_inst(codegen, ir_call->call_target);

	auto llvm_function = static_cast<llvm::Function*>(call_target.address().llvm_pointer());

	auto callee_sig_info = codegen.get_function(call_target_type.type);

	std::vector<llvm::Value*> llvm_arg_values;
	int arg_ind = 0;
	if( callee_sig_info.has_sret_arg() )
	{
		// TODO: Assuming sret is first argument always!
		auto sret_arg_info = callee_sig_info.arg_type(callee_sig_info.sret_arg_index());
		if( lvalue.has_value() )
		{
			llvm_arg_values.push_back(lvalue.value().address().llvm_pointer());
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

	for( auto arg_expr_node : ir_call->args )
	{
		auto arg_expr = codegen_inst(codegen, arg_expr_node);
		auto abi_info = callee_sig_info.arg_type(arg_ind);

		switch( abi_info.attr )
		{
		case LLVMArgABIInfo::UncheckedVarArg:
		case LLVMArgABIInfo::Default:
		{
			auto llvm_value = codegen_operand_expr(codegen, arg_expr);

			llvm_arg_values.push_back(llvm_value);
			break;
		}
		case LLVMArgABIInfo::Value:
		{
			// TODO: This is a bit confusing.
			// Passing struct by values actually passes a pointer.
			// We need to make a copy in a new alloca, and then pass
			// that alloca
			auto address = arg_expr.address();
			auto llvm_arg_expr_value = arg_expr.address().llvm_pointer();
			llvm::AllocaInst* llvm_cpy_alloca =
				codegen.Builder->CreateAlloca(address.llvm_allocated_type(), nullptr);
			auto llvm_size =
				codegen.Module->getDataLayout().getTypeAllocSize(address.llvm_allocated_type());
			auto llvm_align =
				codegen.Module->getDataLayout().getPrefTypeAlign(address.llvm_allocated_type());

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
		return CGExpr::MakeRValue(RValue(llvm_call));
}