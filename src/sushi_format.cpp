#include "ast/ast.h"
#include "format/pretty_print_ast.h"
#include "lexer/Lexer.h"
#include "lexer/TokenCursor.h"
#include "parser/ParseResult.h"
#include "parser/parsers/Parser.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace ast;
using namespace parser;

void
pretty_print(std::vector<Token> const& tokens)
{
	TokenCursor cursor{tokens};
	Parser parse{cursor};
	auto mod_result = parse.parse_module();

	if( mod_result.ok() )
	{
		auto mod = mod_result.unwrap();
		pretty_print_ast(tokens, mod.get());
	}
	else
	{
		mod_result.unwrap_error()->print();
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

	Lexer lex{filedata.c_str()};

	auto lex_result = lex.lex();

	pretty_print(lex_result.tokens);

	return 0;
}