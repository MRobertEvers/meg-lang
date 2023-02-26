#pragma once
#include "SemaLookup.h"
#include "ir/IR.h"
#include "ir/IRBuilder.h"
#include "ir/Name.h"
#include "ir/Types.h"

#include <vector>

namespace sema
{

class Sema
{
	//
	SemaLookup lookup_;
	ir::Types types_;

	ir::IRBuilder builder_;

public:
	Sema();

	SemaLookup& names() { return lookup_; }

	ir::Types& types() { return types_; }

	ir::NameRef create_type(ir::Type type);
	ir::NameRef create_type(ir::NameRef nspace, ir::Type type);

	ir::IRBuilder& builder() { return builder_; }
};
} // namespace sema