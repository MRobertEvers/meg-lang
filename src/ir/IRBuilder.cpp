

#include "IRBuilder.h"

#include "Type.h"

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
IRBuilder::create_fn_decl(NameId name_id, TypeInstance type)
{
	//
	auto fn_decl = new FnDecl(name_id, type);
	create_inst(fn_decl);

	auto var_ref = new VarRef(name_id, type);
	instructions.push_back(var_ref);

	vars.emplace(name_id.index(), var_ref);
	fns.emplace(name_id.index(), fn_decl);

	return fn_decl;
}

FnDef*
IRBuilder::create_fn(std::vector<NameId> args, TypeInstance type)
{
	//
	auto fn = new FnDef(args, type);
	create_inst(fn);

	for( int i = 0; i < type.type->get_member_count(); i++ )
	{
		auto member = type.type->get_member(i);
		auto name_id = args.at(i);
		auto var_ref = new VarRef(name_id, member.type);
		instructions.push_back(var_ref);

		vars.emplace(name_id.index(), var_ref);
	}

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

	auto var_ref = new VarRef(name_id, type);
	instructions.push_back(var_ref);

	vars.emplace(name_id.index(), var_ref);

	return var_ref;
}

VarRef*
IRBuilder::create_var_ref(NameId name_id)
{
	auto iter = vars.find(name_id.index());

	assert(iter != vars.end());

	return iter->second;
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

StringLiteral*
IRBuilder::create_string_literal(std::string val)
{
	// TODO: See comment in const int.
	auto stringlit = new StringLiteral(val);
	instructions.push_back(stringlit);

	return stringlit;
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
IRBuilder::create_basic_block(FnDef* fn)
{
	auto bb = create_basic_block();
	fn->blocks.push_back(bb);

	return bb;
}

FnCall*
IRBuilder::create_call(Inst* call_target, std::vector<Inst*> args, TypeInstance type)
{
	auto ir_call = new FnCall(call_target, args, type);
	create_inst(ir_call);

	return ir_call;
}

Val*
IRBuilder::create_binop(Inst* lhs, Inst* rhs, TypeInstance type)
{
	// TODO: Assert val or varref.
	auto binop = new BinOp(lhs, rhs, type);
	create_inst(binop);

	auto val_ref = new Val(NameId(tmps.size()), type);
	instructions.push_back(val_ref);

	// TODO: Scope temporaries to current function?
	tmps.emplace(tmps.size(), val_ref);

	return val_ref;
}
