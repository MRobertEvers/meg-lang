#pragma once

#include "Name.h"
#include "TypeInstance.h"

#include <optional>
#include <vector>

namespace ir
{

class Inst;
struct BasicBlock
{
	std::vector<Inst*> instructions;

	BasicBlock();
};

struct Module
{
	std::vector<BasicBlock*> blocks;

	Module();
};

enum class InstKind
{
	Bad,
	Alloca,
	Return,
	FnDecl,
	FnDef,
	ConstInt,
	StringLiteral,
	Store,
	VarRef,
	FnCall,
	Val,
	BinOp,
};

struct Inst
{
	ir::TypeInstance type;

	InstKind kind = InstKind::Bad;
	Inst(InstKind kind)
		: kind(kind)
	{}
	Inst(InstKind kind, ir::TypeInstance type)
		: kind(kind)
		, type(type)
	{}
	virtual ~Inst(){};
};

/**
 * @brief Creates an auto variable in the current namespace. Of a given type.
 */
struct Alloca : Inst
{
	NameId name_id;

	Alloca(NameId name_id, TypeInstance type);
};

struct Return : Inst
{
	Inst* operand;

	Return(Inst* operand);
};

/**
 * @brief Creates a declaration of a function and a name with no body. Linker expects to find.
 */
struct FnDecl : Inst
{
	NameId name_id;

	FnDecl(NameId name_id, TypeInstance type);
};

struct FnDef : Inst
{
	std::vector<BasicBlock*> blocks;

	std::vector<NameId> args;

	FnDef(std::vector<NameId> args, TypeInstance type);
};

/**
 * @brief Emits a const int
 */
struct ConstInt : Inst
{
	unsigned long long value;

	ConstInt(unsigned long long);
};

struct StringLiteral : Inst
{
	std::string value;

	StringLiteral(std::string value);
};

struct Store : Inst
{
	Inst* lhs;
	Inst* rhs;

	Store(Inst* lhs, Inst* rhs);
};

/**
 * @brief Causes a variable to be loaded a looked up and returned as an LValue.
 */
struct VarRef : Inst
{
	NameId name_id;

	VarRef(NameId name_id, TypeInstance type);
};

struct FnCall : Inst
{
	Inst* call_target;
	std::vector<Inst*> args;

	FnCall(Inst* call_target, std::vector<Inst*> args, TypeInstance type);
};

/**
 * @brief Causes a temporary to be looked up in the temporaries table.
 */
struct Val : Inst
{
	NameId name_id;

	Val(NameId name_id, TypeInstance type);
};

struct BinOp : Inst
{
	Inst* lhs;
	Inst* rhs;

	BinOp(Inst* lhs, Inst* rhs, TypeInstance type);
};

} // namespace ir