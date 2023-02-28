#include "codegen_switch.h"

#include "../Codegen.h"
#include "cg_discriminations.h"
#include "cg_enum_helpers.h"
#include "common/String.h"
#include "operand.h"

using namespace cg;

static llvm::Value*
cg_switch_expr(CG& codegen, CGExpr expr, sema::TypeInstance expr_type)
{
	if( expr_type.is_enum_type() )
	{
		auto switch_value = cg_enum_nominal(codegen, expr.address());

		auto temp_addr = CGExpr::MakeAddress(switch_value);
		return codegen_operand_expr(codegen, temp_addr);
	}
	else
	{
		return codegen_operand_expr(codegen, expr);
	}
}

CGResult<CGExpr>
cg::codegen_switch(CG& codegen, cg::LLVMFnInfo& fn, ir::IRSwitch* ir_switch)
{
	auto exprr = codegen.codegen_expr(fn, ir_switch->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();

	auto llvm_switch_value = cg_switch_expr(codegen, expr, ir_switch->expr->type_instance);

	auto restore_merge_block = fn.merge_block();
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*codegen.Context, "Merge");
	fn.set_merge_block(llvm_merge_bb);

	llvm::BasicBlock* llvm_default_bb = llvm::BasicBlock::Create(*codegen.Context, "Default");

	auto restore_switch = fn.switch_inst();
	llvm::SwitchInst* llvm_switch =
		codegen.Builder->CreateSwitch(llvm_switch_value, llvm_default_bb, 4);
	fn.set_switch_inst(LLVMSwitchInfo(llvm_switch, expr.address(), llvm_merge_bb, llvm_default_bb));

	auto blockgen = codegen.codegen_block(fn, ir_switch->block);
	if( !blockgen.ok() )
		return blockgen;

	codegen.Builder->SetInsertPoint(llvm_default_bb);
	codegen.Builder->CreateBr(llvm_merge_bb);
	fn.llvm_fn()->getBasicBlockList().push_back(llvm_default_bb);

	codegen.Builder->SetInsertPoint(llvm_merge_bb);
	fn.llvm_fn()->getBasicBlockList().push_back(llvm_merge_bb);

	fn.set_switch_inst(restore_switch);
	fn.set_merge_block(restore_merge_block);

	return CGExpr();
}

CGResult<CGExpr>
cg::codegen_case(CG& codegen, cg::LLVMFnInfo& fn, ir::IRCase* ir_case)
{
	assert(fn.switch_inst().has_value());

	auto switch_info = fn.switch_inst().value();
	auto llvm_switch_inst = switch_info.switch_inst();

	// TODO: backing int type
	llvm::ConstantInt* llvm_const_int =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, ir_case->value, true));

	// TODO: If default block.
	llvm::BasicBlock* llvm_case_bb =
		ir_case->is_default ? switch_info.default_bb()
							: llvm::BasicBlock::Create(
								  *codegen.Context, std::to_string(ir_case->value), fn.llvm_fn());

	llvm_switch_inst->addCase(llvm_const_int, llvm_case_bb);

	codegen.Builder->SetInsertPoint(llvm_case_bb);

	if( ir_case->discriminations.size() != 0 )
	{
		auto temp_address = CGExpr();
		temp_address.add_discrimination(CGExpr::MakeAddress(switch_info.switch_cond()));
		cg_discriminations(codegen, temp_address, ir_case->discriminations);
	}

	auto codegen_result = codegen.codegen_stmt(fn, ir_case->block);
	if( !codegen_result.ok() )
		return codegen_result;

	// TODO: Have fallthrough block stored in switch ctx.
	if( !ir_case->is_default )
		codegen.Builder->CreateBr(switch_info.merge_bb());

	return CGExpr();
}