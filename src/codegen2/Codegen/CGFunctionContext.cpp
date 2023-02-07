#include "CGFunctionContext.h"

using namespace cg;

CGFunctionContext::CGFunctionContext(
	llvm::Function* Fn,
	llvm::Type* Type,
	Vec<llvm::Type*> ArgsTypes,
	sema::Type const* fn_type,
	RetType ret_type)
	: Fn(Fn)
	, FnType(Type)
	, ArgsTypes(ArgsTypes)
	, fn_type(fn_type)
	, ret_type(ret_type)
{
	scopes.push_back(Scope(nullptr));
	current_scope = &scopes.back();
};

void
CGFunctionContext::add_arg_type(llvm::Type* Type)
{
	ArgsTypes.push_back(Type);
}

llvm::Type*
CGFunctionContext::arg_type(int idx)
{
	assert(idx < ArgsTypes.size());
	return ArgsTypes.at(idx);
}

void
CGFunctionContext::add_lvalue(String const& name, LValue lvalue)
{
	current_scope->add_lvalue(name, lvalue);
}