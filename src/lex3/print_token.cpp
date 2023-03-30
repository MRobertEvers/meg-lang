#include "print_token.h"

#include "keywords.h"

#include <iomanip>
#include <iostream>
#include <string>

void
print_token(Token const& token)
{
	TokenView tok = token.view;
	std::string sz{tok.start, tok.start + tok.size};
	std::cout << std::setw(10) << sz << " : " << get_tokentype_string(token.kind) << std::endl;
}