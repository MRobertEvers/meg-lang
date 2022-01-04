#pragma once

#include "token.h"

#include <vector>

class TokenCursor
{
	std::vector<Token> const& _tokens;
	int _index;

public:
	TokenCursor(std::vector<Token> const& toks)
		: _tokens(toks)
		, _index(0){};

	Token peek();

	void adv();

	bool has_tokens();
};