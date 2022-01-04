#include "lexer/Lexer.h"
#include "lexer/TokenCursor.h"
#include "parser/parser.h"
#include "parser/parsers/Parser.h"

#include <iostream>
#include <vector>

using namespace ast;
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