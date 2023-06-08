#include "Function.h"

Arg::Arg(Sym* sym, llvm::Type* type, bool is_sret_, bool is_byval_)
	: sym(sym)
	, type(type)
	, is_sret_(is_sret_)
	, is_byval_(is_byval_)
{}

bool
Arg::is_sret() const
{
	return is_sret_;
}

bool
Arg::is_byval() const
{
	return is_byval_;
}

FunctionYieldPoint::FunctionYieldPoint(
	llvm::BasicBlock* suspend_bb, llvm::BasicBlock* resume_bb, Expr yield_expr)
	: suspend_bb(suspend_bb)
	, resume_bb(resume_bb)
	, yield_expr(yield_expr)
{}

Function::Function(llvm::Function* llvm_func, std::vector<Arg> args, Expr sret)
	: llvm_func(llvm_func)
	, args(args)
	, sret(sret)
{}

Arg&
Function::ir_arg(int i)
{
	if( !sret.is_void() )
		return args.at(i + 1);
	else
		return args.at(i);
}

int
Function::ir_arg_count() const
{
	if( sret.is_void() )
		return args.size();
	else
		return args.size() - 1;
}

int
Function::llvm_arg_index(int ir_index)
{
	if( !sret.is_void() )
		return ir_index + 1;
	else
		return ir_index;
}

Function
Function::FromArgs(llvm::Function* llvm_func, std::vector<Arg> args)
{
	Expr sret = Expr::Empty();
	if( args.size() > 0 && args.at(0).is_sret() )
		sret = Expr(Address(llvm_func->getArg(0), args.at(0).type));

	return Function(llvm_func, args, sret);
}