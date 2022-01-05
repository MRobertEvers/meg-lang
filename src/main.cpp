#include "ast/ast.h"
#include "format/Format.h"
#include "lexer/Lexer.h"
#include "lexer/TokenCursor.h"
#include "parser/parsers/Parser.h"

#include <iostream>
#include <vector>

using namespace ast;
using namespace llvm;

void
codegen(CodegenContext& codegen, std::vector<Token> const& tokens)
{
	Parser parse;

	TokenCursor cursor{tokens};
	auto mod = parse.parse_module(cursor);

	Format fm;
	mod->visit(&fm);
}

int
main()
{
	CodegenContext cg;

	char const buf[] =
		"fn func(a: i32, b: i32): i8 { return 9*8; } fn main(): i8 { return 12*1*3+4; }";
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