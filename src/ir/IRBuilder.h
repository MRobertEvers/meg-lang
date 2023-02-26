#pragma once

#include "IR.h"
#include "Lookup.h"
#include "Name.h"
#include "TypeInstance.h"

#include <map>
#include <vector>

namespace ir
{

class IRBuilder
{
	Lookup lookup_;
	std::vector<Inst*> instructions;
	std::vector<BasicBlock*> blocks;
	BasicBlock* current_block;

	// TODO: Should only be Alloca, or FnDecl
	std::map<int, VarRef*> vars;
	std::map<int, FnDecl*> fns;
	std::map<int, Val*> tmps;

	void create_inst(Inst*);

public:
	IRBuilder();

	BasicBlock* __root() { return blocks[0]; }
	Lookup& lookup() { return lookup_; }

	void set_insert_point(BasicBlock*);

	FnDecl* create_fn_decl(NameId name_id, TypeInstance type);
	FnDef* create_fn(std::vector<NameId> args, TypeInstance type);
	Store* create_store(Inst* lhs, Inst* rhs);
	VarRef* create_alloca(NameId name_id, TypeInstance type);
	VarRef* create_var_ref(NameId name_id);
	Return* create_return(Inst* operand);
	ConstInt* create_const_int(unsigned long long);
	StringLiteral* create_string_literal(std::string val);
	BasicBlock* create_basic_block();
	BasicBlock* create_basic_block(FnDef*);
	// TODO: This should return Val*.
	FnCall* create_call(Inst* call_target, std::vector<Inst*> args, TypeInstance type);
	Val* create_binop(Inst* lhs, Inst* rhs, TypeInstance type);
};
}; // namespace ir