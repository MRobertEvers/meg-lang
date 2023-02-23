#include "cg_division.h"

#include "../Codegen.h"

using namespace cg;

llvm::Value*
cg::cg_division(
	CG& codegen,
	llvm::Value* numerator,
	llvm::Value* denominator,
	sema::TypeInstance numerator_type)
{
	// Clang chooses SDiv vs UDiv based on the numerator
	// If either is float, then is uses float.
	if( codegen.sema.types.is_signed_integer_type(numerator_type) )
		return codegen.Builder->CreateSDiv(denominator, numerator);
	else
		return codegen.Builder->CreateUDiv(denominator, numerator);
}