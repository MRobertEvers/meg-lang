#ifndef KEYWORDS_H_
#define KEYWORDS_H_

#include "Token.h"

TokenKind get_identifier_or_keyword_type(TokenView const& token);

#endif