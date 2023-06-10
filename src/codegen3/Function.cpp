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
{
	for( int i = 0; i < args.size(); i++ )
	{
		auto& arg = args.at(i);
		if( arg.sym )
			ir_args.push_back(i);
	}
}

Arg&
Function::ir_arg(int i)
{
	return args.at(ir_args.at(i));
}

int
Function::ir_arg_count() const
{
	return ir_args.size();
}

int
Function::llvm_arg_index(int ir_index)
{
	return ir_index + (args.size() - ir_args.size());
}

Function
Function::FromArgs(llvm::Function* llvm_func, std::vector<Arg> args)
{
	Expr sret = Expr::Empty();
	if( args.size() > 0 && args.at(0).is_sret() )
		sret = Expr(Address(llvm_func->getArg(0), args.at(0).type));

	return Function(llvm_func, args, sret);
}