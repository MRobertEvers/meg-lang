#include "TokenCursor.h"

#include <string>

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