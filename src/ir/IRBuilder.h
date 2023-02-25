#pragma once

#include "IR.h"

#include <vector>

namespace ir
{

class IRBuilder
{
	std::vector<Inst*> instructions;
	std::vector<BasicBlock*> blocks;
	BasicBlock* current_block;

	void create_inst(Inst*);

public:
	IRBuilder();

	BasicBlock* __root() { return blocks[0]; }

	void set_insert_point(BasicBlock*);

	FnDecl* create_fn_decl(TypeInstance type);
	Function* create_fn(TypeInstance type);
	Alloca* create_alloca(TypeInstance type);
	Return* create_return(Inst* operand);
	ConstInt* create_const_int(unsigned long long);
	BasicBlock* create_basic_block();
	BasicBlock* create_basic_block(Function*);
};
}; // namespace ir