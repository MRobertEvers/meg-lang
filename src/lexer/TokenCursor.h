#pragma once

#include "token.h"

#include <vector>

class ConsumeResult
{
	bool success = false;
	Token const* tok = nullptr;

	ConsumeResult(Token const* tok, bool success)
		: success(success)
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
	Token const* next_token();

public:
	TokenCursor(std::vector<Token> const& toks)
		: _tokens(toks)
		, _index(0){};

	int get_index() const { return _index; }

	bool has_tokens() const;

	/**
	 * @brief Peeks at next non-whitespace/ignored token
	 *
	 * If index is specified, ignored tokens can be peeked.
	 *
	 * @param index
	 * @return Token
	 */
	Token peek(int index = -1) const;

	ConsumeResult consume_if_expected(TokenType expected);

	ConsumeResult consume(TokenType expected);

	// TODO: Template?
	ConsumeResult consume(TokenType expected_one, TokenType expected_two);
	ConsumeResult consume(std::initializer_list<TokenType> expecteds);
};