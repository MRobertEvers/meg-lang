#include "ast3/Ast.h"
#include "lex3/Lex.h"
#include "lex3/Token.h"
#include "lex3/print_token.h"

#include <fstream>
#include <iostream>
#include <sstream>

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

	Lex lex{filedata.c_str()};

	for( Token tok = lex.next(); tok.kind != TokenKind::Eof; tok = lex.next() )
		print_token(tok);

	Ast ast;
}