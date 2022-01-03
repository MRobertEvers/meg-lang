#include "Block.h"

using namespace llvm;
using namespace nodes;

void
Block::codegen()
{
	for( auto& stmt : statements )
	{
		stmt->codegen();
	}

	return;
}