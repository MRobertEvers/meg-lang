#include "LLVMFnInfo.h"

using namespace cg;

LLVMFnInfo::LLVMFnInfo(
	llvm::Function* Fn,
	llvm::Type* Type,
	Vec<ArgumentType> ArgsTypes,
	bool is_var_arg,
	sema::Type const* fn_type,
	RetType ret_type)
	: Fn(Fn)
	, FnType(Type)
	, ArgsTypes(ArgsTypes)
	, is_var_arg(is_var_arg)
	, fn_type(fn_type)
	, ret_type(ret_type)
{
	scopes.push_back(Scope(nullptr));
	current_scope = &scopes.back();
};

void
LLVMFnInfo::add_arg_type(ArgumentType Type)
{
	ArgsTypes.push_back(Type);
}

ArgumentType
LLVMFnInfo::arg_type(int idx)
{
	assert(idx < ArgsTypes.size());
	return ArgsTypes.at(idx);
}

void
LLVMFnInfo::add_lvalue(String const& name, LValue lvalue)
{
	current_scope->add_lvalue(name, lvalue);
}