

#include "IRBuilder.h"

using namespace ir;

IRBuilder::IRBuilder()
	: current_block(new BasicBlock())
{
	this->blocks.push_back(current_block);
}

void
IRBuilder::create_inst(Inst* inst)
{
	instructions.push_back(inst);
	current_block->instructions.push_back(inst);
}

void
IRBuilder::set_insert_point(BasicBlock* bb)
{
	current_block = bb;
}

FnDecl*
IRBuilder::create_fn_decl(TypeInstance type)
{
	//
	auto fn_decl = new FnDecl(type);
	create_inst(fn_decl);

	return fn_decl;
}

Function*
IRBuilder::create_fn(TypeInstance type)
{
	//
	auto fn = new Function(type);
	create_inst(fn);

	return fn;
}

Alloca*
IRBuilder::create_alloca(TypeInstance type)
{
	//
	auto alloca = new Alloca(type);
	create_inst(alloca);

	return alloca;
}

Return*
IRBuilder::create_return(Inst* operand)
{
	//
	auto ret = new Return(operand);
	create_inst(ret);

	return ret;
}

ConstInt*
IRBuilder::create_const_int(unsigned long long val)
{
	//
	auto constint = new ConstInt(val);
	create_inst(constint);

	return constint;
}

BasicBlock*
IRBuilder::create_basic_block()
{
	//
	auto bb = new BasicBlock();

	blocks.push_back(bb);

	return bb;
}

BasicBlock*
IRBuilder::create_basic_block(Function* fn)
{
	auto bb = create_basic_block();
	fn->blocks.push_back(bb);

	return bb;
}