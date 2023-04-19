#include "Codegen.h"

#include "Expr.h"
#include <llvm/IR/IRBuilder.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

Codegen::Codegen()
	: context(std::make_unique<llvm::LLVMContext>())
	, mod(std::make_unique<llvm::Module>("this_module", *context))
	, builder(std::make_unique<llvm::IRBuilder<>>(*context))
{}

void
Codegen::print()
{
	std::string str;
	llvm::raw_string_ostream out(str);

	mod->print(out, nullptr);

	std::cout << str;
}

Codegen
Codegen::codegen(HirNode* module)
{
	Codegen gen;

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

Expr
Codegen::codegen_function(HirNode* hir_func)
{
	//
	HirFunc& func = hir_cast<HirFunc>(hir_func);
	HirFuncProto& proto = hir_cast<HirFuncProto>(func.proto);

	llvm::FunctionType* llvm_fn_ty =
		llvm::FunctionType::get(llvm::Type::getVoidTy(*context), {}, false);

	llvm::Function* llvm_fn =
		llvm::Function::Create(llvm_fn_ty, llvm::Function::ExternalLinkage, "main", mod.get());

	llvm::BasicBlock* llvm_entry_bb = llvm::BasicBlock::Create(*context, "entry", llvm_fn);
	builder->SetInsertPoint(llvm_entry_bb);

	codegen_block(func.body);

	return Expr::Empty();
}

Expr
Codegen::codegen_block(HirNode* hir_block)
{
	HirBlock& block = hir_cast<HirBlock>(hir_block);

	for( auto& stmt : block.statements )
		codegen_statement(stmt);

	return Expr::Empty();
}

Expr
Codegen::codegen_statement(HirNode* hir_stmt)
{
	switch( hir_stmt->kind )
	{
	case HirNodeKind::Return:
		return codegen_return(hir_stmt);
	default:
		return Expr::Empty();
	}
}

Expr
Codegen::codegen_expr(HirNode* hir_expr)
{
	switch( hir_expr->kind )
	{
	case HirNodeKind::NumberLiteral:
		return codegen_number_literal(hir_expr);
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
	else
	{
		RValue rvalue = expr.rvalue();
		return rvalue.llvm_pointer();
	}
}