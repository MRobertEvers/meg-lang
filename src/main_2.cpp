#include "ast2/Ast.h"
#include "ast2/AstGen.h"
#include "ast2/AstNode.h"
#include "ast2/AstTags.h"
#include "codegen2/Codegen.h"
#include "common/OwnPtr.h"
#include "lexer/Lexer.h"
#include "lexer/TokenCursor.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "sema2/Sema2.h"
#include "sema2/SemaGen.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace sema;
using namespace cg;
using namespace ast;
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

int
main(int argc, char* argv[])
{
	if( argc == 1 )
	{
		std::cout << "Please specify a file" << std::endl;
		return -1;
	}

	auto filepath = argv[argc - 1];

	std::ifstream file{filepath};
	if( !file.good() )
	{
		std::cout << "Could not open file " << filepath << std::endl;
		return -1;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string filedata = buffer.str();

	std::cout << filedata << std::endl;

	Lexer lex{filedata.c_str()};

	auto lex_result = lex.lex();

	// Lexer::print_tokens(lex_result.tokens);

	TokenCursor cursor{lex_result.tokens};
	Ast ast;
	AstGen gen{ast, cursor};
	auto result = gen.parse();
	if( !result.ok() )
		result.unwrap_error()->print();

	sema::Sema2 sema;

	auto sema_result = sema::sema_module(sema, result.unwrap());
	// auto sema_result = sema.sema(result.unwrap());
	if( !sema_result.ok() )
		sema_result.unwrap_error()->print();

	CG cg{sema};

	auto cgr = cg.codegen_module(sema_result.unwrap());
	if( !cgr.ok() )
	{
		cgr.unwrap_error()->print();
		// return 0;
	}

	// llvm::FunctionType* llvm_fn_ty_inner = llvm::FunctionType::get(
	// 	llvm::Type::getVoidTy(*cg.Context), {llvm::Type::getInt32Ty(*cg.Context)}, false);
	// llvm::Function* llvm_fn_inner = llvm::Function::Create(
	// 	llvm_fn_ty_inner, llvm::Function::ExternalLinkage, "inner", cg.Module.get());

	// llvm::BasicBlock* llvm_inner_send_entry_block =
	// 	llvm::BasicBlock::Create(*cg.Context, "entry", llvm_fn_inner);
	// cg.Builder->SetInsertPoint(llvm_inner_send_entry_block);
	// cg.Builder->CreateRetVoid();

	// llvm::FunctionType* llvm_fn_ty_outer = llvm::FunctionType::get(
	// 	llvm::Type::getVoidTy(*cg.Context), {llvm::Type::getInt32Ty(*cg.Context)}, false);
	// llvm::Function* llvm_fn_outer = llvm::Function::Create(
	// 	llvm_fn_ty_outer, llvm::Function::ExternalLinkage, "outer", cg.Module.get());

	// llvm::BasicBlock* llvm_outer_send_entry_block =
	// 	llvm::BasicBlock::Create(*cg.Context, "entry", llvm_fn_outer);
	// cg.Builder->SetInsertPoint(llvm_outer_send_entry_block);
	// // cg.Builder->CreateAlloca(llvm_fn_inner->getArg(0)->getType());
	// // cg.Builder->CreateLoad(llvm_fn_inner->getArg(0)->getType(), llvm_fn_inner->getArg(0));
	// cg.Builder->CreateCall(llvm_fn_inner, {llvm_fn_outer->getArg(0)});
	// cg.Builder->CreateRetVoid();

	std::string Str;
	raw_string_ostream OS(Str);

	cg.Module->print(OS, nullptr);

	std::cout << Str;

	return emit(cg.Module.get());
}