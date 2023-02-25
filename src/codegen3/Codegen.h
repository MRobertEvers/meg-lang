#pragma once
#include "Codegen/LLVMFnSigInfo.h"
#include "ir/IR.h"
#include "ir/Type.h"
#include "sema3/Sema.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <map>
#include <memory>
#include <optional>

namespace cg
{

struct CG
{
public:
	std::unique_ptr<llvm::LLVMContext> Context;
	std::unique_ptr<llvm::IRBuilder<>> Builder;
	std::unique_ptr<llvm::Module> Module;

	std::map<ir::Type const*, llvm::Type*> types;
	std::map<std::string, LLVMFnSigInfo> functions;

	CG(sema::Sema& sema);

	std::optional<llvm::Type*> find_type(ir::Type const*);
};

} // namespace cg