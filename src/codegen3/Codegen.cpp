#include "Codegen.h"

using namespace cg;

static void
establish_llvm_builtin_types(CG& cg, ir::Types& types, std::map<ir::Type const*, llvm::Type*>& lut)
{
	// lut.emplace(types.u8_type(), llvm::Type::getInt8Ty(*cg.Context));
	// lut.emplace(types.u16_type(), llvm::Type::getInt16Ty(*cg.Context));
	// lut.emplace(types.u32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.i8_type(), llvm::Type::getInt8Ty(*cg.Context));
	// lut.emplace(types.i16_type(), llvm::Type::getInt16Ty(*cg.Context));
	lut.emplace(types.i32_type(), llvm::Type::getInt32Ty(*cg.Context));
	lut.emplace(types.void_type(), llvm::Type::getVoidTy(*cg.Context));
}

CG::CG(sema::Sema& sema)
{
	Context = std::make_unique<llvm::LLVMContext>();
	Module = std::make_unique<llvm::Module>("this_module", *Context);
	Builder = std::make_unique<llvm::IRBuilder<>>(*Context);

	establish_llvm_builtin_types(*this, sema.types(), this->types);
}

std::optional<llvm::Type*>
CG::find_type(ir::Type const* ty)
{
	auto t = types.find(ty);
	if( t != types.end() )
		return t->second;
	else
		return std::optional<llvm::Type*>();
}