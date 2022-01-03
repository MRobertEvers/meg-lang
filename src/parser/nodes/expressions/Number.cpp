#include "Number.h"

#include <llvm/IR/Constants.h>

using namespace llvm;
using namespace nodes;

Value*
Number::codegen()
{
	return ConstantInt::get(*TheContext, APInt(32, Val, true));
}