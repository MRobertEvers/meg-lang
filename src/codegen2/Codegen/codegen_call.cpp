#include "codegen_call.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

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

	if( ir_call->call_target->type == ir::IRExprType::Id )
	{
		ir::IRId* id = ir_call->call_target->expr.id;
		std::string name_str = id->name.name().name_str();
		if( name_str == "@SizeOf" )
		{
			sema::TypeInstance sizeof_type = ir_call->args->args.at(0)->type_instance;

			llvm::Type* llvm_sizeof_type = get_type(codegen, sizeof_type).unwrap();

			auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(llvm_sizeof_type);

			llvm::Value* llvm_const_int = llvm::ConstantInt::get(
				*codegen.Context, llvm::APInt(32, llvm_size.getFixedSize(), true));

			return CGExpr::MakeRValue(RValue(llvm_const_int));
		}
	}

	// TODO: EmitCallee like clang.
	CGExpr callee_expr = codegen.codegen_expr(fn, ir_call->call_target).unwrap();
	llvm::Value* llvm_callee = callee_expr.address().llvm_pointer();
	// auto llvm_function = static_cast<llvm::Function*>(expr.address().llvm_pointer());

	auto iter_callee = codegen.Functions.find(ir_call->call_target->type_instance.type);
	assert(iter_callee != codegen.Functions.end());
	LLVMFnSigInfo callee_sig_info = iter_callee->second;
	llvm::Function* llvm_function = callee_sig_info.llvm_fn;

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
				codegen.builder_alloca(sret_arg_info.llvm_type->getPointerElementType(), ".dummy");
			llvm_arg_values.push_back(llvm_sret_alloca);
		}
		arg_ind += 1;
	}

	if( callee_sig_info.sema_fn_ty->is_this_call() )
	{
		llvm_arg_values.push_back(llvm_callee);
	}

	for( auto arg_expr_node : ir_call->args->args )
	{
		auto arg_exprr = codegen.codegen_expr(fn, arg_expr_node);
		if( !arg_exprr.ok() )
			return arg_exprr;

		auto arg_expr = arg_exprr.unwrap();
		LLVMArgABIInfo abi_info = callee_sig_info.arg_type(arg_ind);

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
			auto llvm_value = codegen_operand_expr(codegen, arg_expr);

			llvm_arg_values.push_back(llvm_value);
			break;
		}
		case LLVMArgABIInfo::ValueRef:
		{
			// TODO: This is a bit confusing.
			// Passing struct by values actually passes a pointer.
			// We need to make a copy in a new alloca, and then pass
			// that alloca
			if( arg_expr.is_address() )
			{
				// TODO: Clang explicitly creates a memcpy for pass-by-value structs...
				// I couldn't tell if this was needed.
				auto address = arg_expr.address();
				llvm::Value* llvm_arg_expr_value = arg_expr.address().llvm_pointer();

				llvm::AllocaInst* llvm_cpy_alloca =
					codegen.builder_alloca(address.llvm_allocated_type());
				auto llvm_size =
					codegen.Module->getDataLayout().getTypeAllocSize(address.llvm_allocated_type());
				auto llvm_align =
					codegen.Module->getDataLayout().getPrefTypeAlign(address.llvm_allocated_type());

				codegen.Builder->CreateMemCpy(
					llvm_cpy_alloca, llvm_align, llvm_arg_expr_value, llvm_align, llvm_size);

				llvm_arg_values.push_back(llvm_cpy_alloca);
				break;
			}
			else
			{
				// Canibalize rvalue.
				llvm_arg_values.push_back(arg_expr.rvalue().llvm_pointer());
				break;
			}
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