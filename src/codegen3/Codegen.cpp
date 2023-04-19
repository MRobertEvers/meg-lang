#include "Codegen.h"

#include "Expr.h"
#include "emit.h"
#include <llvm/IR/IRBuilder.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Codegen::Codegen(SymBuiltins& builtins)
	: context(std::make_unique<llvm::LLVMContext>())
	, mod(std::make_unique<llvm::Module>("this_module", *context))
	, builder(std::make_unique<llvm::IRBuilder<>>(*context))
{
	tys.emplace(builtins.i8_ty, llvm::Type::getInt8Ty(*context));
	tys.emplace(builtins.i16_ty, llvm::Type::getInt16Ty(*context));
	tys.emplace(builtins.i32_ty, llvm::Type::getInt32Ty(*context));
	tys.emplace(builtins.i64_ty, llvm::Type::getInt64Ty(*context));
	tys.emplace(builtins.u8_ty, llvm::Type::getInt8Ty(*context));
	tys.emplace(builtins.u16_ty, llvm::Type::getInt16Ty(*context));
	tys.emplace(builtins.u32_ty, llvm::Type::getInt32Ty(*context));
	tys.emplace(builtins.u64_ty, llvm::Type::getInt8Ty(*context));

	tys.emplace(builtins.void_ty, llvm::Type::getVoidTy(*context));
	tys.emplace(builtins.bool_ty, llvm::Type::getInt1Ty(*context));
}

void
Codegen::print()
{
	std::string str;
	llvm::raw_string_ostream out(str);

	mod->print(out, nullptr);

	std::cout << str;
}

int
Codegen::emit()
{
	return ::emit(mod.get());
}

Codegen
Codegen::codegen(SymBuiltins& builtins, HirNode* module)
{
	Codegen gen(builtins);

	gen.codegen_module(module);

	return gen;
}

Expr
Codegen::codegen_module(HirNode* hir_mod)
{
	HirModule& mod = hir_cast<HirModule>(hir_mod);

	for( auto stmt : mod.statements )
		codegen_item(stmt);

	return Expr::Empty();
}

Expr
Codegen::codegen_item(HirNode* hir_item)
{
	switch( hir_item->kind )
	{
	case HirNodeKind::Func:
		return codegen_function(hir_item);
	default:
		return Expr::Empty();
	}
}

static TyFunc const&
func_ty(Sym* sym)
{
	SymFunc& func = sym_cast<SymFunc>(sym);
	TyFunc const& func_ty = ty_cast<TyFunc>(func.ty);

	return func_ty;
}
static std::string
func_name(Sym* sym)
{
	// Eventually mangle names.
	return func_ty(sym).name;
}

static llvm::Function::LinkageTypes
linkage(HirFuncProto::Linkage link)
{
	switch( link )
	{
	case HirFuncProto::Linkage::Extern:
		return llvm::Function::ExternalLinkage;
	default:
		return llvm::Function::PrivateLinkage;
	}
}

Expr
Codegen::codegen_function(HirNode* hir_func)
{
	//
	HirFunc& func = hir_cast<HirFunc>(hir_func);
	HirFuncProto& proto = hir_cast<HirFuncProto>(func.proto);

	TyFunc const& ty_func = func_ty(proto.sym);

	llvm::Type* ret_ty = get_type(ty_func.rt_qty);
	std::vector<llvm::Type*> arg_tys;
	for( auto arg_qty : ty_func.args_qtys )
	{
		llvm::Type* arg_ty = get_type(arg_qty);
		arg_tys.push_back(arg_ty);
	}

	llvm::FunctionType* llvm_fn_ty =
		llvm::FunctionType::get(ret_ty, arg_tys, proto.var_arg == HirFuncProto::VarArg::VarArg);

	llvm::Function* llvm_fn =
		llvm::Function::Create(llvm_fn_ty, linkage(proto.linkage), func_name(proto.sym), mod.get());

	auto emplaced = funcs.emplace(proto.sym, Function(llvm_fn));
	current_func = &emplaced.first->second;

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	for( int i = 0; i < arg_tys.size(); i++ )
	{
		HirNode* arg = proto.parameters.at(i);
		llvm::Type* ty = arg_tys.at(i);
		HirId& arg_id = hir_cast<HirId>(arg);

		llvm::Value* alloca = builder->CreateAlloca(ty);
		builder->CreateStore(llvm_fn->getArg(i), alloca);
		vars.emplace(arg_id.sym, Address(alloca, ty));
	}

	codegen_block(func.body);

	vars.clear();

	return Expr::Empty();
}

Expr
Codegen::codegen_call(HirNode* hir_call)
{
	HirCall& call = hir_cast<HirCall>(hir_call);
	assert(call.kind != HirCall::CallKind::Invalid);

	switch( call.kind )
	{
	case HirCall::CallKind::BinOp:
		return codegen_binop(hir_call);
	case HirCall::CallKind::Static:
		return codegen_func_call(hir_call);
	case HirCall::CallKind::BuiltIn:
		return codegen_builtin(hir_call);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_func_call(HirNode* hir_call)
{
	HirCall& call = hir_cast<HirCall>(hir_call);
	assert(call.kind == HirCall::CallKind::Static);
}

Expr
Codegen::codegen_binop(HirNode* hir_call)
{
	HirCall& call = hir_cast<HirCall>(hir_call);
	assert(call.kind == HirCall::CallKind::BinOp);
	assert(call.args.size() == 2);
	Expr lhs_expr = codegen_expr(call.args.at(0));
	Expr rhs_expr = codegen_expr(call.args.at(1));

	llvm::Value* llvm_lhs = codegen_eval(lhs_expr);
	llvm::Value* llvm_rhs = codegen_eval(rhs_expr);

	switch( call.op )
	{
	case BinOp::Add:
		return Expr(RValue(builder->CreateNSWAdd(llvm_lhs, llvm_rhs)));
	case BinOp::Gt:
		return Expr(RValue(builder->CreateICmpSGT(llvm_lhs, llvm_rhs)));
	case BinOp::Lt:
		return Expr(RValue(builder->CreateICmpSLT(llvm_lhs, llvm_rhs)));
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_builtin(HirNode* hir_call)
{
	HirCall& call = hir_cast<HirCall>(hir_call);
	assert(call.kind == HirCall::CallKind::BuiltIn);

	switch( call.builtin )
	{
	case HirCall::BuiltinKind::IntCast:
		return codegen_intcast(hir_call);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_intcast(HirNode* hir_call)
{
	HirCall& call = hir_cast<HirCall>(hir_call);
	assert(call.kind == HirCall::CallKind::BuiltIn);
	assert(call.builtin == HirCall::BuiltinKind::IntCast);

	HirId& id = hir_cast<HirId>(call.args.at(0));
	Ty const* ty = sym_cast<SymType>(id.sym).ty;
	llvm::Type* dest = get_type(ty);

	Expr value_expr = codegen_expr(call.args.at(1));
	llvm::Value* value = codegen_eval(value_expr);
	assert(value);

	TyInt const& ty_int = ty_cast<TyInt>(ty);
	switch( ty_int.sign )
	{
	case TyInt::Sign::Signed:
		// S is for Signed-Extend
		return RValue(builder->CreateSExt(value, dest));
	case TyInt::Sign::Unsigned:
		// Z is for Zero-Extend
		return RValue(builder->CreateZExt(value, dest));
	case TyInt::Sign::Any:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_block(HirNode* hir_block)
{
	HirBlock& block = hir_cast<HirBlock>(hir_block);

	for( auto& stmt : block.statements )
		codegen_expr(stmt);

	return Expr::Empty();
}

Expr
Codegen::codegen_if(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);

	if( if_nod.cond_else )
		return codegen_if_phi(hir_if);
	else
		return codegen_if_chain(hir_if);
}

Expr
Codegen::codegen_if_chain(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);
	assert(!if_nod.cond_else);

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_cond_bb = builder->GetInsertBlock();
	llvm::BasicBlock* llvm_then_bb = nullptr;
	llvm::BasicBlock* llvm_else_bb = nullptr;
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*context, "");

	for( int i = 0; i < if_nod.elsifs.size(); i++ )
	{
		HirIf::CondThen& cond_then = if_nod.elsifs.at(i);

		Expr cond = codegen_expr(cond_then.cond);
		llvm::Value* cond_val = codegen_eval(cond);

		llvm_then_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
		llvm_else_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
		builder->CreateCondBr(cond_val, llvm_then_bb, llvm_else_bb);

		builder->SetInsertPoint(llvm_then_bb);

		codegen_expr(cond_then.then);
		builder->CreateBr(llvm_merge_bb);

		builder->SetInsertPoint(llvm_else_bb);
	}

	if( if_nod.else_node )
	{
		codegen_expr(if_nod.else_node);
		builder->CreateBr(llvm_merge_bb);
	}

	llvm_merge_bb->insertInto(current_func->llvm_func);
	builder->SetInsertPoint(llvm_merge_bb);

	return Expr::Empty();
}

/**
 * Used for short circuiting boolean expressions
 * Only supports 1 chain.
 *
 * e.g.
 * bool val = a || b
 *
 * yields something like
 *
 * %1 = a
 * if (!val)
 *  %2 = b
 * val = phi %1 %2
 *
 *
 * @return Expr
 */
Expr
Codegen::codegen_if_phi(HirNode* hir_if)
{
	HirIf& if_nod = hir_cast<HirIf>(hir_if);
	assert(!if_nod.else_node && if_nod.cond_else && if_nod.elsifs.size() == 1);

	llvm::Function* llvm_fn = current_func->llvm_func;
	llvm::BasicBlock* llvm_cond_bb = builder->GetInsertBlock();
	llvm::BasicBlock* llvm_else_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);
	llvm::BasicBlock* llvm_merge_bb = llvm::BasicBlock::Create(*context, "", llvm_fn);

	HirIf::CondThen& cond_then = if_nod.elsifs.at(0);
	Expr cond = codegen_expr(cond_then.cond);
	llvm::Value* cond_val = codegen_eval(cond);

	builder->CreateCondBr(cond_val, llvm_merge_bb, llvm_else_bb);

	builder->SetInsertPoint(llvm_else_bb);
	Expr then = codegen_expr(cond_then.then);
	llvm::Value* then_val = codegen_eval(then);
	builder->CreateBr(llvm_merge_bb);

	builder->SetInsertPoint(llvm_merge_bb);
	llvm::PHINode* phi = builder->CreatePHI(llvm::Type::getInt1Ty(*context), 2);
	phi->addIncoming(cond_val, llvm_cond_bb);
	phi->addIncoming(then_val, llvm_else_bb);

	return RValue(phi);
}

Expr
Codegen::codegen_var(HirNode* hir_id)
{
	HirId& var = hir_cast<HirId>(hir_id);

	auto iter_vars = vars.find(var.sym);
	assert(iter_vars != vars.end());
	return Expr(iter_vars->second);
}

Expr
Codegen::codegen_expr(HirNode* hir_expr)
{
	switch( hir_expr->kind )
	{
	case HirNodeKind::NumberLiteral:
		return codegen_number_literal(hir_expr);
	case HirNodeKind::Call:
		return codegen_call(hir_expr);
	case HirNodeKind::Id:
		return codegen_var(hir_expr);
	case HirNodeKind::Return:
		return codegen_return(hir_expr);
	case HirNodeKind::If:
		return codegen_if(hir_expr);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_return(HirNode* hir_return)
{
	HirReturn& ret = hir_cast<HirReturn>(hir_return);

	Expr expr = codegen_expr(ret.expr);

	if( expr.is_void() )
	{
		builder->CreateRetVoid();
	}
	else
	{
		auto llvm_value = codegen_eval(expr);
		assert(!llvm_value->getType()->isStructTy());
		builder->CreateRet(llvm_value);
	}

	return Expr::Empty();
}

Expr
Codegen::codegen_number_literal(HirNode* hir_nl)
{
	HirNumberLiteral nl = hir_cast<HirNumberLiteral>(hir_nl);
	llvm::Value* llvm_const_int = llvm::ConstantInt::get(*context, llvm::APInt(32, nl.value, true));

	return Expr(RValue(llvm_const_int, llvm_const_int->getType()));
}

llvm::Value*
Codegen::codegen_eval(Expr expr)
{
	if( expr.is_address() )
	{
		Address address = expr.address();
		return builder->CreateLoad(address.llvm_allocated_type(), address.llvm_pointer());
	}
	else if( expr.is_rvalue() )
	{
		RValue rvalue = expr.rvalue();
		return rvalue.llvm_pointer();
	}
	else
	{
		assert("Invalid eval.");
		return nullptr;
	}
}

llvm::Type*
Codegen::get_type(QualifiedTy qty)
{
	llvm::Type* base = get_type(qty.ty);

	for( int i = 0; i < qty.indirection; i++ )
		base = base->getPointerTo();

	return base;
}

llvm::Type*
Codegen::get_type(Ty const* ty)
{
	return tys[ty];
}