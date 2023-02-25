#pragma once
#include "Name.h"
#include "SemaLookup.h"
#include "Types.h"
#include "ir/IR.h"

#include <vector>

namespace sema
{

class Sema
{
	//
	SemaLookup lookup_;
	Types types_;

	std::vector<ir::LIRInst*> out_;

public:
	Sema();

	SemaLookup& names() { return lookup_; }

	NameRef create_type(Type type);
	NameRef create_type(NameRef nspace, Type type);

	void emit(ir::LIRInst* inst) { out_.push_back(inst); }
};
} // namespace sema