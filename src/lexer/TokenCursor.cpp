#include "TokenCursor.h"

#include <string>

ConsumeResult
TokenCursor::consume(TokenType expected)
{
	if( auto tok = next_token(); tok->type == expected )
	{
		_index += 1;
		return ConsumeResult{tok};
	}
	else
	{
		return ConsumeResult::fail(tok);
	}
}

ConsumeResult
TokenCursor::consume(TokenType expected_one, TokenType expected_two)
{
	auto tok = next_token();
	if( tok->type == expected_one || tok->type == expected_two )
	{
		_index += 1;
		return ConsumeResult{tok};
	}
	else
	{
		return ConsumeResult::fail(tok);
	}
}

Token const*
TokenCursor::next_token() const
{
	if( _index >= _tokens.size() )
	{
		return nullptr;
	}

	return &_tokens[_index];
}

Token
TokenCursor::peek()
{
	if( _index >= _tokens.size() )
	{
		throw new std::string{"oops"};
	}
	return _tokens[_index];
}

void
TokenCursor::adv()
{
	_index += 1;
	if( _index > _tokens.size() )
	{
		throw new std::string{"What?"};
	}
}

bool
TokenCursor::has_tokens()
{
	return _index < _tokens.size();
}