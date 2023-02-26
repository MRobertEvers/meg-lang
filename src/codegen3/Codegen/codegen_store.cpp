#include "codegen_store.h"

#include "ast2/Ast.h"
#include "codegen_inst.h"
#include "operand.h"

using namespace cg;

CGExpr
cg::codegen_store(CG& codegen, ir::Store* ir_store)
{
	auto lhs_expr = codegen_inst(codegen, ir_store->lhs);
	auto rhs_expr = codegen_inst(codegen, ir_store->rhs);

	auto lhs = lhs_expr.address().llvm_pointer();
	auto rhs = codegen_operand_expr(codegen, rhs_expr);

	codegen.Builder->CreateStore(rhs, lhs);

	return CGExpr();
}