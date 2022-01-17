#include "TokenCursor.h"

#include <string>

bool
TokenCursor::has_tokens() const
{
	return _index <= _tokens.size();
}

Token
TokenCursor::peek(int index) const
{
	if( index != -1 )
	{
		return _tokens[index];
	}
	else
	{
		int ind = _index;
		while( _tokens[ind].type == TokenType::line_comment )
		{
			ind++;
			if( ind >= _tokens.size() )
			{
				return Token{};
			}
		}
		return _tokens[ind];
	}
}

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

ConsumeResult
TokenCursor::consume(std::initializer_list<TokenType> expecteds)
{
	auto tok = next_token();
	for( auto& exp : expecteds )
	{
		if( tok->type == exp )
		{
			_index += 1;
			return ConsumeResult{tok};
		}
	}
	return ConsumeResult::fail(tok);
}

Token const*
TokenCursor::next_token()
{
	if( _index >= _tokens.size() )
	{
		return nullptr;
	}

	while( _tokens[_index].type == TokenType::line_comment )
	{
		_index++;
		if( _index >= _tokens.size() )
		{
			return nullptr;
		}
	}

	return &_tokens[_index];
}

ConsumeResult
TokenCursor::consume_if_expected(TokenType expected)
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
