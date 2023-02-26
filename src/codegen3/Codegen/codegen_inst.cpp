#include "codegen_inst.h"

#include "codegen_alloca.h"
#include "codegen_fn_sig_info.h"
#include "codegen_function.h"
#include "codegen_return.h"
#include "codegen_store.h"
#include "codegen_var_ref.h"
#include "lookup.h"
#include <llvm/IR/IRBuilder.h>

using namespace cg;

static CGExpr
codegen_bb(CG& codegen, ir::BasicBlock* bb)
{
	for( auto inst : bb->instructions )
		codegen_inst(codegen, inst);

	return CGExpr();
}
// static LLVMFnSigInfo
// codegen_fn_proto(CG& codegen, ir::FnDecl* fn_decl)
// {
// 	return CGExpr();
// }

static CGExpr
codegen_const_int(CG& codegen, ir::ConstInt* ci)
{
	llvm::Value* llvm_const_int =
		llvm::ConstantInt::get(*codegen.Context, llvm::APInt(32, ci->value, true));

	return CGExpr::MakeRValue(RValue(llvm_const_int, llvm_const_int->getType()));
}

CGExpr
cg::codegen_inst(CG& codegen, ir::Inst* inst)
{
	switch( inst->kind )
	{
	case ir::InstKind::Return:
		return codegen_return(codegen, (ir::Return*)inst);
	case ir::InstKind::FnDecl:
		return codegen_function_proto(codegen, (ir::FnDecl*)inst);
	case ir::InstKind::Function:
		return codegen_function(codegen, (ir::Function*)inst);
	case ir::InstKind::ConstInt:
		return codegen_const_int(codegen, (ir::ConstInt*)inst);
	case ir::InstKind::Alloca:
		return codegen_decl_var(codegen, (ir::Alloca*)inst);
	case ir::InstKind::Store:
		return codegen_store(codegen, (ir::Store*)inst);
	case ir::InstKind::VarRef:
		return codegen_var_ref(codegen, (ir::VarRef*)inst);
	default:
		assert(0 && "Unimplemented Instruction.");
	}

	return CGExpr();
}