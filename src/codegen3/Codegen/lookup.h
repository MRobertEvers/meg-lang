
#pragma once

#include "ir/Type.h"
#include <llvm/IR/IRBuilder.h>

#include <optional>

namespace cg
{

class CG;

std::optional<llvm::Type*> get_base_type(CG& cg, ir::Type const* ty);
std::optional<llvm::Type*> get_type(CG& cg, ir::TypeInstance ty);

} // namespace cg