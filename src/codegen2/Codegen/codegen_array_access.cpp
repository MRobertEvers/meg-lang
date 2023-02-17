#include "codegen_array_access.h"

#include "../Codegen.h"
#include "lookup.h"
#include "operand.h"

using namespace cg;

CGResult<CGExpr>
cg::codegen_array_access(CG& codegen, cg::LLVMFnInfo& fn, ir::IRArrayAccess* ir_array_access)
{
	auto array_targetr = codegen.codegen_expr(fn, ir_array_access->array_target);
	if( !array_targetr.ok() )
		return array_targetr;
	auto array_target = array_targetr.unwrap();

	auto llvm_target_type = array_target.address().llvm_allocated_type();
	auto llvm_target_value = array_target.address().llvm_pointer();

	auto exprr = codegen.codegen_expr(fn, ir_array_access->expr);
	if( !exprr.ok() )
		return exprr;
	auto expr = exprr.unwrap();
	auto llvm_expr_value = codegen_operand_expr(codegen, expr);

	llvm::Constant* zero =
		llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(*codegen.Context));
	llvm::Value* indices[] = {zero, llvm_expr_value};

	auto llvm_array_value =
		codegen.Builder->CreateInBoundsGEP(llvm_target_type, llvm_target_value, indices);

	return CGExpr::MakeAddress(
		LLVMAddress(llvm_array_value, llvm_target_type->getArrayElementType()));
}