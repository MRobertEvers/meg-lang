#ifndef KEYWORDS_H_
#define KEYWORDS_H_

#include "Token.h"

TokenKind get_identifier_or_keyword_type(TokenView const& token);

char const* get_tokentype_string(TokenKind const& tok_kind);

#endif