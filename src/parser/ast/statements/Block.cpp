#include "Block.h"

using namespace llvm;
using namespace ast;

void
Block::codegen(CodegenContext& codegen)
{
	for( auto& stmt : statements )
	{
		stmt->codegen(codegen);
	}

	return;
}