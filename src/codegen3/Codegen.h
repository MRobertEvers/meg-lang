#pragma once
#include "Codegen/LLVMAddress.h"
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

	// Names are needed here for external functions and mangling
	std::vector<ir::Name> names;

	// We may need to look up function by type since
	// some function targets may not have a name (e.g. function pointer.)
	std::map<ir::Type const*, int> functions_types;
	std::map<int, LLVMFnSigInfo> functions;

	std::map<int, LLVMAddress> vars;

	CG(sema::Sema& sema);

	std::optional<llvm::Type*> find_type(ir::Type const*);

	void add_function(int, LLVMFnSigInfo);
	LLVMFnSigInfo get_function(ir::Type const*);
};

} // namespace cg