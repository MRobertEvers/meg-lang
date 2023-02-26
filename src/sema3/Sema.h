#pragma once
#include "ir/IR.h"
#include "ir/IRBuilder.h"
#include "ir/Lookup.h"
#include "ir/Name.h"
#include "ir/Types.h"

#include <vector>

namespace sema
{

class Sema
{
	//
	ir::Types types_;

	ir::IRBuilder builder_;

public:
	Sema();

	ir::Lookup& names() { return builder_.lookup(); }

	ir::Types& types() { return types_; }

	ir::NameRef create_type(ir::Type type);
	ir::NameRef create_type(ir::NameRef nspace, ir::Type type);

	ir::IRBuilder& builder() { return builder_; }
};
} // namespace sema