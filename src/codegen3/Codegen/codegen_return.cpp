#include "codegen_return.h"

#include "../Codegen.h"
#include "codegen_inst.h"
#include "operand.h"

using namespace cg;

CGExpr
cg::codegen_return(CG& codegen, ir::Return* ir_return)
{
	if( ir_return->operand == nullptr )
	{
		codegen.Builder->CreateRetVoid();
		return CGExpr();
	}

	auto operand = codegen_inst(codegen, ir_return->operand);

	// if( fn.has_sret_arg() )
	// {
	// 	auto llvm_value = operand.address().llvm_pointer();
	// 	auto sret_arg = fn.sret();
	// 	auto llvm_sret_value = sret_arg.lvalue.address().llvm_pointer();
	// 	auto llvm_sret_type = sret_arg.lvalue.address().llvm_allocated_type();

	// 	// TODO: Compute alignment from member
	// 	auto llvm_size = codegen.Module->getDataLayout().getTypeAllocSize(llvm_sret_type);
	// 	auto llvm_align = codegen.Module->getDataLayout().getPrefTypeAlign(llvm_sret_type);

	// 	codegen.Builder->CreateMemCpy(
	// 		llvm_sret_value, llvm_align, llvm_value, llvm_align, llvm_size);
	// 	codegen.Builder->CreateRetVoid();
	// }
	// else
	{
		auto llvm_value = codegen_operand_expr(codegen, operand);
		assert(!llvm_value->getType()->isStructTy());
		codegen.Builder->CreateRet(llvm_value);
	}

	return CGExpr();
}