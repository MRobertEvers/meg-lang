#pragma once

#include "Name.h"
#include "TypeInstance.h"

#include <optional>
#include <vector>

namespace ir
{

enum class InstKind
{
	Bad,
	Alloca,
	Return,
	FnDecl,
	Function,
	ConstInt,
	Store,
	VarRef
};

struct Inst
{
	InstKind kind = InstKind::Bad;
	Inst(InstKind kind)
		: kind(kind)
	{}
	virtual ~Inst(){};
};

struct BasicBlock
{
	std::vector<Inst*> instructions;

	BasicBlock();
};

/**
 * @brief Creates an auto variable in the current namespace. Of a given type.
 */
struct Alloca : Inst
{
	ir::TypeInstance type;
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
	ir::TypeInstance type;

	FnDecl(TypeInstance type);
};

struct Function : Inst
{
	ir::TypeInstance type;
	std::vector<BasicBlock*> blocks;

	Function(TypeInstance type);
};

/**
 * @brief Emits a const int
 */
struct ConstInt : Inst
{
	unsigned long long value;

	ConstInt(unsigned long long);
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

	VarRef(NameId name_id);
};

} // namespace ir