#include "ast3/Ast.h"
#include "ast3/Parser.h"
#include "lex3/Lex.h"
#include "lex3/Token.h"
#include "lex3/print_token.h"

#include <fstream>
#include <iostream>
#include <sstream>

void
print_tokens(char const* input)
{
	Lex lex{input};

	for( Token tok = lex.next(); tok.kind != TokenKind::Eof; tok = lex.next() )
		print_token(tok);
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

	print_tokens(filedata.c_str());

	Cursor cursor(filedata.c_str());
	Ast ast;
	ParseResult<AstNode*> root = Parser::parse(ast, cursor);
	if( !root.ok() )
		root.unwrap_error()->print();

	return 0;
}