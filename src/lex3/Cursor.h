#pragma once

#include "Lex.h"
#include "Token.h"

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

struct Cursor
{
	Lex cursor;

	Cursor(char const* input);

	ConsumeResult consume_if_expected(TokenType expected);
	ConsumeResult consume(TokenType expected);
	ConsumeResult consume(std::initializer_list<TokenType> expecteds);
}