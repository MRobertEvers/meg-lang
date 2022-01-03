#include "Return.h"

using namespace llvm;
using namespace nodes;

Value*
Return::codegen()
{
	for( auto& stmt : statements )
	{
		stmt->codegen();
	}

	return nullptr;
}