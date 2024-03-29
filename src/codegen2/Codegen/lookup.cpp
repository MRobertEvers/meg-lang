#include "lookup.h"

#include "../Codegen.h"
#include "LLVMFnSigInfo.h"

using namespace cg;

CGResult<llvm::Type*>
cg::get_base_type(CG& cg, sema::Type const* ty)
{
	auto maybe_llvm_type = cg.find_type(ty);
	if( !maybe_llvm_type.has_value() )
		return CGError("Missing Type!");

	return maybe_llvm_type.value();
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, sema::TypeInstance ty)
{
	auto maybe_llvm_type = get_base_type(cg, ty.type);
	if( !maybe_llvm_type.ok() )
		return maybe_llvm_type;

	auto type = maybe_llvm_type.unwrap();

	// TODO: Try to use opaque pointers? We have to keep track of the pointer
	// type ourselves.
	for( int i = 0; i < ty.indirection_level; i++ )
		type = type->getPointerTo();

	if( ty.is_array_type() )
		return llvm::ArrayType::get(type, ty.array_size);
	else
		return type;
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, ir::IRTypeDeclaraor* decl)
{
	return get_type(cg, decl->type_instance);
}

CGResult<llvm::Type*>
cg::get_type(CG& cg, ir::IRValueDecl* decl)
{
	return get_type(cg, decl->type_decl);
}

// Get LValue?
std::optional<LValue>
cg::get_value(CG& cg, String const& name)
{
	auto iter_value = cg.values.find(name);
	if( iter_value != cg.values.end() )
		return iter_value->second;
	else
		return std::optional<LValue>();
}