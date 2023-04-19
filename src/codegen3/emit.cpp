#include "emit.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;
int
emit(llvm::Module* Module)
{
	auto& Mod = *Module;

	auto CPU = "generic";
	auto Features = "";

	InitializeNativeTarget();
	InitializeNativeTargetAsmParser();
	InitializeNativeTargetAsmPrinter();
	TargetOptions opt;
	auto RM = Optional<Reloc::Model>();

	// auto TargetTriple = sys::getDefaultTargetTriple();
	std::string TargetTriple = "aarch64-app-darwin";
	// std::cout << "Target: " << TargetTriple << std::endl;

	std::string Error;
	auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
	if( !Target )
	{
		errs() << Error;
		return -1;
	}

	auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

	Mod.setDataLayout(TheTargetMachine->createDataLayout());
	Mod.setTargetTriple(TargetTriple);

	auto Filename = "output.o";
	std::error_code EC;
	raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

	if( EC )
	{
		errs() << "Could not open file: " << EC.message();
		return -1;
	}

	legacy::PassManager pass;
	auto FileType = CGFT_ObjectFile;

	if( TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType) )
	{
		errs() << "TheTargetMachine can't emit a file of this type";
		return -1;
	}

	pass.run(Mod);
	dest.flush();
	return 0;
}