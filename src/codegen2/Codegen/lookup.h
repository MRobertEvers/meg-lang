
#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "../LValue.h"
#include "LLVMFnSigInfo.h"
#include "sema2/IR.h"
#include "sema2/Name.h"
#include "sema2/Scope.h"
#include "sema2/Sema2.h"

#include <optional>

namespace cg
{

class CG;

CGResult<llvm::Type*> get_base_type(CG& cg, sema::Type const* ty);

CGResult<llvm::Type*> get_type(CG& cg, sema::TypeInstance ty);

CGResult<llvm::Type*> get_type(CG& cg, ir::IRTypeDeclaraor* decl);

CGResult<llvm::Type*> get_type(CG& cg, ir::IRValueDecl* decl);

std::optional<LValue> get_value(CG&, sema::NameId id);
} // namespace cg