#pragma once

#include "Lex.h"
#include "Token.h"

#include <deque>
#include <vector>

class ConsumeResult
{
	bool success = false;
	Token const* tok = nullptr;

private:
	ConsumeResult(Token const* tok, bool success)
		: success(success)
		, tok(tok)
	{}

public:
	ConsumeResult(Token const* tok)
		: ConsumeResult(tok, true)
	{}

	bool ok() const { return success; }
	Token token() const { return *tok; }

	static ConsumeResult Fail(Token const* tok) { return ConsumeResult(tok, false); }
};

class Cursor
{
	Lex lex;
	std::vector<Token> tokens;

	// Tokens peeked.
	std::deque<Token*> queue;

public:
	Cursor(char const* input);

	// TODO: Enforce only already seen tokens.
	bool at_end() const;
	Token token_at(int ind);

	ConsumeResult consume_if_expected(TokenKind expected);
	ConsumeResult consume(TokenKind expected);
	ConsumeResult consume(std::initializer_list<TokenKind> expecteds);

private:
	Token* current();
	Token* consume(bool consume);
};