#ifndef KEYWORDS_H_
#define KEYWORDS_H_

#include "token.h"

TokenType get_identifier_or_keyword_type(Token const& token);

char const* get_tokentype_string(Token const& token);

#endif