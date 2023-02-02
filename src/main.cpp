#include "ast2/AstNode.h"
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

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace ast;
using namespace llvm;

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

	Lexer::print_tokens(lex_result.tokens);

	Ast ast;

	auto fn = ast.Fn(Span{}, nullptr, nullptr);
}