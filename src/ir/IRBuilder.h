#pragma once

#include "IR.h"
#include "Name.h"

#include <map>
#include <vector>

namespace ir
{

class IRBuilder
{
	std::vector<Inst*> instructions;
	std::vector<BasicBlock*> blocks;
	BasicBlock* current_block;

	// TODO: Should only be Alloca, or FnDecl
	std::map<int, VarRef*> vars;
	std::map<int, FnDecl*> fns;

	void create_inst(Inst*);

public:
	IRBuilder();

	BasicBlock* __root() { return blocks[0]; }

	void set_insert_point(BasicBlock*);

	FnDecl* create_fn_decl(NameId name_id, TypeInstance type);
	Function* create_fn(TypeInstance type);
	Store* create_store(Inst* lhs, Inst* rhs);
	VarRef* create_alloca(NameId name_id, TypeInstance type);
	VarRef* create_var_ref(NameId name_id);
	Return* create_return(Inst* operand);
	ConstInt* create_const_int(unsigned long long);
	StringLiteral* create_string_literal(std::string val);
	BasicBlock* create_basic_block();
	BasicBlock* create_basic_block(Function*);
	FnCall* create_call(Inst* call_target, std::vector<Inst*> args, TypeInstance type);
};
}; // namespace ir