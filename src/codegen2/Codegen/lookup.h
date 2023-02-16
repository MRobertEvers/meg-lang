
#pragma once

#include "../CGExpr.h"
#include "../CGResult.h"
#include "../LValue.h"
#include "LLVMFnSigInfo.h"
#include "common/Vec.h"
#include "sema2/IR.h"
#include "sema2/Scope.h"
#include "sema2/Sema2.h"
#include <llvm-c/Core.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <optional>

namespace cg
{

class CG;

CGResult<llvm::Type*> get_base_type(CG& cg, sema::TypeInstance ty);

CGResult<llvm::Type*> get_type(CG& cg, sema::TypeInstance ty);

CGResult<llvm::Type*> get_type(CG& cg, ir::IRTypeDeclaraor* decl);

CGResult<llvm::Type*> get_type(CG& cg, ir::IRValueDecl* decl);

std::optional<LValue> get_value(CG&, String const&);
} // namespace cg