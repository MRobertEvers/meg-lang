#include "operand.h"

#include "../Codegen.h"

using namespace cg;

llvm::Value*
cg::codegen_operand_expr(CG& codegen, CGExpr& result)
{
	if( result.is_address() )
	{
		auto address = result.address();
		return codegen.Builder->CreateLoad(address.llvm_allocated_type(), address.llvm_pointer());
	}
	else
	{
		auto rvalue = result.rvalue();
		return rvalue.llvm_pointer();
	}
}