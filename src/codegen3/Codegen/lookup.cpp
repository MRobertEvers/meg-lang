#include "lookup.h"

#include "../Codegen.h"
#include "LLVMFnSigInfo.h"

using namespace cg;

std::optional<llvm::Type*>
cg::get_base_type(CG& cg, ir::Type const* ty)
{
	auto maybe_llvm_type = cg.find_type(ty);
	if( !maybe_llvm_type.has_value() )
		return std::optional<llvm::Type*>();

	return maybe_llvm_type.value();
}

std::optional<llvm::Type*>
cg::get_type(CG& cg, ir::TypeInstance ty)
{
	auto maybe_llvm_type = get_base_type(cg, ty.type);
	if( !maybe_llvm_type.has_value() )
		return maybe_llvm_type;

	auto type = maybe_llvm_type.value();

	// TODO: Try to use opaque pointers? We have to keep track of the pointer
	// type ourselves.
	for( int i = 0; i < ty.indirection_level; i++ )
		type = type->getPointerTo();

	if( ty.is_array_type() )
		return llvm::ArrayType::get(type, ty.array_size);
	else
		return type;
}
