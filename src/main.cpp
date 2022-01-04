#include "lexer/Lexer.h"
#include "lexer/TokenCursor.h"
#include "parser/parser.h"
#include "parser/parsers/Parser.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>

#include <iostream>
#include <map>
#include <stdio.h>
#include <string.h>
#include <vector>

/// BinopPrecedence - This holds the precedence for each binary operator that is
/// defined.

using namespace nodes;
using namespace llvm;

void
codegen(CodegenContext& codegen, std::vector<Token> const& tokens)
{
	Parser parse;
	std::vector<IExpressionNode*> asts;
	asts.reserve(30);

	TokenCursor cursor{tokens};
	auto mod = parse.parse_module(cursor);
	if( !mod )
	{
		return;
	}
	mod->codegen(codegen);
}

int
main()
{
	CodegenContext cg;

	char const buf[] = "fn func(): i8 { return 9*8; }";
	Lexer lex{buf};

	auto tokens = lex.lex();

	Lexer::print_tokens(tokens);

	codegen(cg, tokens);

	std::string Str;
	raw_string_ostream OS(Str);

	cg.Module->print(OS, nullptr);

	std::cout << Str;

	return 0;
}