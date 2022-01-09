#include "ParseResult.h"

#include "common/String.h"

#include <iostream>

void
ParseError::print() const
{
	std::cout << "Parse Error:\n";
	std::cout << "\t" << error << "\n";
	std::cout << "while parsing\n" << String{token.start, token.size} << "\n";
	std::cout << String(token.size, '-') << "\n|\n"
			  << "here" << std::endl;
}