#pragma once

#include "../Name.h"
#include "../TypeInstance.h"
#include "ActionResult.h"

#include <optional>
#include <vector>

namespace ir
{
struct LIRInst
{
	virtual ~LIRInst(){};
};

struct BasicBlock
{
	std::vector<LIRInst*> instructions;
};

//
struct FnDecl : LIRInst
{
	enum Linkage
	{
		Default,
		Extern,
	};

	Linkage linkage = Default;

	sema::NameRef name;

	FnDecl(sema::NameRef name, Linkage linkage);
};

struct VarDecl : LIRInst
{
	sema::NameRef name;
	sema::TypeInstance type;
};

struct Return : LIRInst
{
	std::optional<ActionResult> operand;
};
} // namespace ir