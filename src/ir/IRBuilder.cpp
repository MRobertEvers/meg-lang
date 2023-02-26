

#include "IRBuilder.h"

using namespace ir;

IRBuilder::IRBuilder()
	: current_block(new BasicBlock())
{
	this->blocks.push_back(current_block);

	// TODO: Stable inst.
	instructions.reserve(1000);
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

Store*
IRBuilder::create_store(Inst* lhs, Inst* rhs)
{
	auto fn = new Store(lhs, rhs);
	create_inst(fn);

	return fn;
}

VarRef*
IRBuilder::create_alloca(NameId name_id, TypeInstance type)
{
	//
	auto alloca = new Alloca(name_id, type);
	create_inst(alloca);

	vars.emplace(name_id.index(), alloca);

	auto var_ref = new VarRef(name_id);
	create_inst(var_ref);

	return var_ref;
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
	// TODO: These should just be created freestanding.
	// Ex. See LLVM, we have to create a ConstInt separate
	// from the builder and pass that value around.
	// We want to do the same here because the ConstInt
	// instruction doesn't do anything by itself.
	auto constint = new ConstInt(val);
	// create_inst(constint);
	instructions.push_back(constint);

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