#include "Number.h"

#include <llvm/IR/Constants.h>

using namespace llvm;
using namespace ast;

// Value*
// Number::codegen(CodegenContext& codegen)
// {
// 	return ConstantInt::get(*codegen.Context, APInt(32, Val, true));
// }