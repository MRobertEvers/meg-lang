#include "Token.h"

TokenView::TokenView()
{}

TokenView::TokenView(char const* start)
	: start(start)
	, size(0)
{}

Token::Token(){};
Token::Token(TokenView view, TokenKind kind)
	: view(view)
	, kind(kind)
{}