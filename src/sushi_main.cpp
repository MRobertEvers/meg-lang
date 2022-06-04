#include "ast/ast.h"
#include "codegen/Codegen.h"
#include "common/OwnPtr.h"
#include "format/pretty_print_ast.h"
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
#include "parser/ParseResult.h"
#include "parser/parsers/Parser.h"
#include "sema/Sema.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace ast;
using namespace llvm;
using namespace parser;

int
gen_code(codegen::Codegen& cg, std::vector<Token> const& tokens)
{
	TokenCursor cursor{tokens};
	Parser parse{cursor};
	auto mod_result = parse.parse_module();

	// sema::Sema sema;

	if( mod_result.ok() )
	{
		auto mod = mod_result.unwrap();

		// mod->visit(&sema);
		// if( sema.is_errored() )
		// {
		// 	sema.print_err();
		// }
		// else
		{
			mod->visit(&cg);
		}

		return 0;
	}
	else
	{
		mod_result.unwrap_error()->print();
		return -1;
	}
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

	codegen::Codegen cg;

	Lexer lex{filedata.c_str()};

	auto lex_result = lex.lex();

	auto code = gen_code(cg, lex_result.tokens);

	if( code != 0 )
	{
		return -1;
	}

	std::string Str;
	raw_string_ostream OS(Str);

	auto& Mod = *cg.Module;

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