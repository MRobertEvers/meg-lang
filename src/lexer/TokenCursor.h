#pragma once

#include "token.h"

#include <vector>

class ConsumeResult
{
	bool success = false;
	Token const* tok = nullptr;

	ConsumeResult(Token const* tok, bool success)
		: success(false)
		, tok(tok)
	{}

public:
	ConsumeResult(Token const* tok)
		: ConsumeResult(tok, true)
	{}

	bool ok() const { return success; }

	Token as() const { return *tok; }

	// TODO: Panic if !success?
	Token unwrap() const { return *tok; }

	static ConsumeResult fail(Token const* tok) { return ConsumeResult(tok, false); }
};

class TokenCursor
{
	std::vector<Token> const& _tokens;
	int _index;

private:
	Token const* next_token() const;

public:
	TokenCursor(std::vector<Token> const& toks)
		: _tokens(toks)
		, _index(0){};

	Token peek();

	void adv();

	bool has_tokens();

	ConsumeResult consume(TokenType expected);

	// TODO: Template?
	ConsumeResult consume(TokenType expected_one, TokenType expected_two);
};